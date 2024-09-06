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
    try {
      const updateMessage = await this.#getMessageData(message, ws);
      this._sendMessageToClient(ws, updateMessage);
    } catch (error) {
      logMessage(
        "Error reading object from Realm: ".concat(error),
        MessageType.ERROR,
      );
      this._sendMessageToClient(ws, { error: "Error reading object" });
    }
  }

  async write(message, ws) {
    try {
      const mediaElement = await this.#getMediaElement(message, ws);
      let nodes = message.node ? [message.node] : message.nodes;
      const dataPoints = [];

      if (mediaElement) {
        this.realm.write(() => {
          nodes.forEach(({ name, value }) => {
            const prop = this._transformDatapointsWithUnderscores(name);
            dataPoints.push(name);
            mediaElement[prop] = value;
          });
        });
      } else {
        const dataPointId = mediaElementsParams[message.tree].dataPointId;
        let document = { _id: uuidv4(), [dataPointId]: message.id };

        nodes.forEach(({ name, value }) => {
          const prop = this._transformDatapointsWithUnderscores(name);
          dataPoints.push(name);
          document[prop] = value;
        });
        const databaseName = mediaElementsParams[message.tree].databaseName;

        this.realm.write(() => {
          this.realm.create(databaseName, document);
        });
      }

      const updateMessage = await this.#getMessageData(message, ws);
      this._sendMessageToClient(ws, updateMessage);
    } catch (error) {
      logMessage(
        "Error writing object changes in Realm: ".concat(error),
        MessageType.ERROR,
      );
      this._sendMessageToClient(ws, { error: "Error writing object changes" });
    }
  }

  async subscribe(message, ws) {
    try {
      const mediaElement = await this.#getMediaElement(message, ws);
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

          const mediaElementToSubscribe = await this.realm.objectForPrimaryKey(
            databaseName,
            objectId,
          );

          if (mediaElementToSubscribe) {
            const listener = (mediaElementToSubscribe, changes) =>
              this.#onMediaElementChange(
                mediaElementToSubscribe,
                changes,
                {
                  id: id,
                  tree: tree,
                  uuid: uuid,
                },
                ws,
              );

            mediaElementToSubscribe.addListener(listener);

            // Store the listener so we can remove it later
            this.listeners.get(ws).set(id, {
              objectId: objectId,
              mediaElement: mediaElementToSubscribe,
              listener: listener,
            });

            this._sendMessageToClient(ws, {
              success: `Subscribed to ${dataPointId}: '${id}'`,
            });

            logWithColor(
              `Subscription added! Amount Clients: ${this.listeners.size}`,
              COLORS.GREY,
            );
          } else {
            this._sendMessageToClient(ws, {
              error: "Object could not be subscribed",
            });
          }
        } else {
          this._sendMessageToClient(ws, {
            success: `Subscription was already done to ${dataPointId}: '${id}'`,
          });
        }
      } else {
        this._sendMessageToClient(ws, { error: "Object not found" });
      }
    } catch (error) {
      this._sendMessageToClient(ws, {
        error: "Error subscribing to object changes",
      });
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

        this._sendMessageToClient(ws, {
          success: `Unsubscribed to ${dataPointId}: '${id}'`,
        });
      } else {
        this._sendMessageToClient(ws, {
          error: `No subscription found for VIN: ${id}`,
        });
      }
    } else {
      this._sendMessageToClient(ws, {
        error: `No subscription found for VIN: ${id}`,
      });
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
   * Asynchronously retrieves media element, processes it, and creates an updated message.
   *
   * @param {Object} message - The message object containing the data to be processed.
   * @param {WebSocket} ws - The WebSocket connection used for communication.
   * @returns {Promise<Object>} - A promise that resolves to the updated message object.
   * @private
   */
  async #getMessageData(message, ws) {
    const mediaElement = await this.#getMediaElement(message, ws);
    logWithColor(
      `Media Element: \n ${JSON.stringify(mediaElement)}`,
      COLORS.GREY,
    );
    if (mediaElement) {
      const responseNodes = this.#parseReadResponse(message, mediaElement);
      return this._createUpdateMessage(message, responseNodes);
    } else {
      throw new Error(`No data found with the Id: ${message.id}`);
    }
  }

  /**
   * Asynchronously retrieves a media element from the database based on the provided message.
   *
   * @param {Object} message - The message containing the id and tree information.
   * @param {WebSocket} ws - The WebSocket connection to send error messages if needed.
   * @returns {Promise<Object>} - The media element object from the database.
   */
  async #getMediaElement(message, ws) {
    try {
      const { id, tree } = message;
      const { databaseName, dataPointId } = mediaElementsParams[tree];
      return await this.realm
        .objects(databaseName)
        .filtered(`${dataPointId} = '${id}'`)[0];
    } catch (error) {
      this._sendMessageToClient(ws, { error: "Error reading object" });
    }
  }

  /**
   * Handles changes to a media element and sends update messages to clients.
   * @param {mediaElement} mediaElement - The media element that has changed.
   * @param {object} changes - An object containing information about the changes.
   * @param {messageHeader} messageHeader - The header information for the message.
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

  /**
   * Parses the response from a read event.
   *
   * @param {Object} message - The message object containing node or nodes information.
   * @param {Object} queryResponseObj - The query response object containing values to be mapped.
   * @returns {Object} - A data object with keys from the message nodes and values from the query response.
   */
  #parseReadResponse(message, queryResponseObj) {
    const data = [];
    const nodes = message.node ? [message.node] : message.nodes;
    nodes.forEach((node) => {
      const prop = this._transformDatapointsWithUnderscores(node.name);
      data.push({
        name: node.name,
        value: queryResponseObj[prop],
      });
    });
    return data;
  }
}

module.exports = RealmDBHandler;
