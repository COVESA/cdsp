const Realm = require("realm");
const Handler = require("../../handler");
const {
  mediaElementsParams,
  databaseConfig,
} = require("../config/database-params");
const realmConfig = require("../config/realm-configuration");
const { v4: uuidv4 } = require("uuid");
const {
  logMessage,
  logWithColor,
  MessageType,
  COLORS,
} = require("../../../../utils/logger");
const {
  createErrorMessage,
} = require("../../../../utils/error-message-helper");

/**
 * Parses the response from a media element change event.
 *
 * @param {Object} changes - The object containing the changed properties.
 * @param {Object} mediaElement - The media element object with updated properties.
 * @returns {Array} An array of objects, each containing the name and value of a changed property.
 */
function parseOnMediaElementChangeResponse(changes, mediaElement) {
  return changes.changedProperties.map((prop) => ({
    name: prop,
    value: mediaElement[prop],
  }));
}

class RealmDBHandler extends Handler {
  constructor() {
    super();
    this.realm = null;
    this._sendMessageToClients = null;
    this.listeners = new Map();
  }

  async authenticateAndConnect(sendMessageToClients) {
    try {
      this._sendMessageToClients = sendMessageToClients;

      const app = new Realm.App({ id: databaseConfig.realmAppId });
      const credentials = Realm.Credentials.apiKey(databaseConfig.realmApiKey);
      const user = await app.logIn(credentials);
      console.info("Successfully authenticated to RealmDB");

      const supportedDataPoints = this._getSupportedDataPoints();
      const realmConfigObj = realmConfig(user, supportedDataPoints);
      this.realm = await Realm.open(realmConfigObj);
      console.info("Connection established successfully");

      for (const [key, value] of Object.entries(mediaElementsParams)) {
        try {
          const databaseName = value.databaseName;
          await this.realm.objects(databaseName).subscribe();
          console.info(`Subscribed to the database ${key}: ${databaseName}`);
        } catch (error) {
          logMessage(
            "Error subscribing databases: ".concat(error),
            MessageType.ERROR,
          );
        }
      }
    } catch (error) {
      logMessage(
        "Failed to authenticate with Realm: ".concat(error),
        MessageType.ERROR,
      );
    }
  }

  async read(message, ws) {
    if (this.#areNodesValid(message, ws)) {
      try {
        const updateMessage = await this.#getMessageData(message);
        this._sendMessageToClient(ws, updateMessage);
      } catch (error) {
        logMessage(
          `Error reading object from Realm: ${error.message}`,
          MessageType.ERROR,
        );
        this._sendMessageToClient(
          ws,
          createErrorMessage("read", 404, JSON.parse(error.message)),
        );
      }
    }
  }

  async write(message, ws) {
    if (this.#areNodesValid(message, ws)) {
      try {
        const mediaElement = await this.#getMediaElement(message);
        const nodes = message.node ? [message.node] : message.nodes;

        const transformAndAssign = (element, nodes) => {
          nodes.forEach(({ name, value }) => {
            const prop = this._transformDataPointsWithUnderscores(name);
            element[prop] = value;
          });
        };

        this.realm.write(() => {
          if (mediaElement) {
            // Write on existing media element
            transformAndAssign(mediaElement, nodes);
          } else {
            // Create a new media element
            const dataPointId = mediaElementsParams[message.tree].dataPointId;
            const document = { _id: uuidv4(), [dataPointId]: message.id };
            transformAndAssign(document, nodes);
            const databaseName = mediaElementsParams[message.tree].databaseName;
            this.realm.create(databaseName, document);
          }
        });

        // Read the updated media element
        await this.read(message, ws);
      } catch (error) {
        const errorMessage = `Schema is not compatible for that media element: ${error.message}`;
        logMessage(errorMessage, MessageType.ERROR);
        this._sendMessageToClient(
          ws,
          createErrorMessage("write", 404, errorMessage),
        );
      }
    }
  }

  async subscribe(message, ws) {
    try {
      const mediaElement = await this.#getMediaElement(message);

      if (mediaElement) {
        const objectId = mediaElement._id;
        const { id, tree, uuid } = message;
        const { databaseName, dataPointId } = mediaElementsParams[tree];

        if (!this.listeners.has(ws)) {
          this.listeners.set(ws, new Map());
        }

        if (!this.listeners.get(ws).has(id)) {
          logWithColor(
            `Subscribing element for user '${uuid}': Object ID: ${objectId} with ${dataPointId}: '${id}' on ${databaseName}`,
            COLORS.GREY,
          );

          const listener = (mediaElement, changes) =>
            this.#onMediaElementChange(
              mediaElement,
              changes,
              {
                id: id,
                tree: tree,
                uuid: uuid,
              },
              ws,
            );

          mediaElement.addListener(listener);

          // Store the listener so we can remove it later
          this.listeners.get(ws).set(id, {
            objectId: objectId,
            mediaElement: mediaElement,
            listener: listener,
          });

          this._sendMessageToClient(
            ws,
            this._createSubscribeMessage("subscribe", message, "succeed"),
          );

          logWithColor(
            `Subscription added! Amount Clients: ${this.listeners.size}`,
            COLORS.GREY,
          );
        } else {
          this._sendMessageToClient(
            ws,
            createErrorMessage(
              "subscribe",
              400,
              `Subscription already done to ${dataPointId}: '${id}'`,
            ),
          );
        }
      } else {
        this._sendMessageToClient(
          ws,
          createErrorMessage("subscribe", 400, "Object not found"),
        );
      }
    } catch (error) {
      this._sendMessageToClient(
        ws,
        createErrorMessage(
          "subscribe",
          503,
          "Subscription process could not finish, try again",
        ),
      );
    }
  }

  async unsubscribe(message, ws) {
    const { id, tree, uuid } = message;
    const { databaseName, dataPointId } = mediaElementsParams[tree];

    if (this.listeners.has(ws)) {
      const wsListeners = this.listeners.get(ws);
      if (wsListeners.has(id)) {
        const listener = wsListeners.get(id);
        logWithColor(
          `Unsubscribing element for user '${uuid}': Object ID: ${listener.objectId} with ${dataPointId}: '${id}' on ${databaseName}`,
          COLORS.GREY,
        );
        listener.mediaElement.removeListener(listener.listener);
        wsListeners.delete(id);

        if (wsListeners.size === 0) {
          this.listeners.delete(ws);
        }

        this._sendMessageToClient(
          ws,
          this._createSubscribeMessage("unsubscribe", message, "succeed"),
        );
      } else {
        this._sendMessageToClient(
          ws,
          createErrorMessage(
            "unsubscribe",
            400,
            `No subscription found for VIN: ${id}`,
          ),
        );
      }
    } else {
      this._sendMessageToClient(
        ws,
        createErrorMessage(
          "unsubscribe",
          400,
          `No subscription found for this client`,
        ),
      );
    }
    logWithColor(
      `Subscription removed! Amount Clients: ${this.listeners.size}`,
      COLORS.GREY,
    );
  }

  async unsubscribe_client(ws) {
    this.listeners.delete(ws);
    logWithColor(
      `All client subscriptions removed! Amount Clients: ${this.listeners.size}`,
      COLORS.GREY,
    );
  }

  /**
   * Validates the nodes in a message against the schema of a media element.
   *
   * @param {Object} message - The message object containing details for the request.
   * @param {WebSocket} ws - The WebSocket object for communication.
   * @returns {boolean} - Returns true if all nodes are valid against the schema, otherwise false.
   */
  #areNodesValid(message, ws) {
    const { type, tree } = message;
    const { databaseName } = mediaElementsParams[tree];

    // Get the schema for the mediaElement to check if fields are valid
    const mediaElementSchema = this.realm.schema.find(
      (schema) => schema.name === databaseName,
    );

    const errorData = this._validateNodesAgainstSchema(
      message,
      mediaElementSchema.properties,
    );

    if (errorData) {
      logMessage(
        `Error validating message nodes against schema: ${JSON.stringify(errorData)}`,
        MessageType.ERROR,
      );
      this._sendMessageToClient(
        ws,
        createErrorMessage(`${type}`, 404, errorData),
      );

      return false;
    }
    return true;
  }

  /**
   * Asynchronously processes a message to fetch and handle media data.
   *
   * @param {Object} message - The message object containing details for the request.
   * @returns {Promise<Object>} - Returns a promise that resolves to an updated message object.
   * @throws {Error} - Throws an error if no media element for the message ID is found.
   * @private
   */
  async #getMessageData(message) {
    const mediaElement = await this.#getMediaElement(message);

    if (!mediaElement) {
      throw new Error(`\"No data found with the Id: ${message.id}\"`);
    }

    logWithColor(
      `Media Element: \n ${JSON.stringify(mediaElement)}`,
      COLORS.GREY,
    );

    const responseNodes = this.#parseReadResponse(message, mediaElement);
    return this._createUpdateMessage(message, responseNodes);
  }

  /**
   * Parses the response from a read event.
   *
   * @param {Object} message - The message object containing node or nodes information.
   * @param {Object} queryResponseObj - The query response object containing values to be mapped.
   * @returns {Object} - A data object with keys from the message nodes and values from the query response.
   * @private
   */
  #parseReadResponse(message, queryResponseObj) {
    const data = [];
    const nodes = message.node ? [message.node] : message.nodes;
    nodes.forEach((node) => {
      const prop = this._transformDataPointsWithUnderscores(node.name);
      data.push({
        name: node.name,
        value: queryResponseObj[prop],
      });
    });
    return data;
  }

  /**
   * Asynchronously retrieves a media element from the database based on the provided message.
   *
   * @param {Object} message - The message containing the id and tree information.
   * @returns {Promise<Object>} - The media element object from the database.
   * @private
   */
  async #getMediaElement(message) {
    try {
      const { id, tree } = message;
      const { databaseName, dataPointId } = mediaElementsParams[tree];
      return await this.realm
        .objects(databaseName)
        .filtered(`${dataPointId} = '${id}'`)[0];
    } catch (error) {
      logMessage(
        `Error trying to get media element from Realm: ${error.message}`,
        MessageType.ERROR,
      );
    }
  }

  /**
   * Handles changes to a media element and sends update messages to clients.
   * @param {mediaElement} mediaElement - The media element that has changed.
   * @param {object} changes - An object containing information about the changes.
   * @param {{tree, id, uuid}} messageHeader - The header information for the message.
   * @param {WebSocket} ws - The WebSocket object for communication.
   * @private
   */
  #onMediaElementChange(mediaElement, changes, messageHeader, ws) {
    logMessage(
      "Media element changed",
      MessageType.RECEIVED,
      `Web-Socket Connection Event Received`,
    );
    if (changes.deleted) {
      logMessage(changes.deleted, COLORS.YELLOW, "MediaElement is deleted");
    } else {
      if (changes.changedProperties.length > 0) {
        const responseNodes = parseOnMediaElementChangeResponse(
          changes,
          mediaElement,
        );
        const updateMessage = this._createUpdateMessage(
          messageHeader,
          responseNodes,
        );
        this._sendMessageToClient(ws, updateMessage);
      }
    }
  }
}

module.exports = RealmDBHandler;
