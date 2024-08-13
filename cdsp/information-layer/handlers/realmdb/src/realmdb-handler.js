const Realm = require("realm");
const Handler = require("../../handler");
const config = require("../config/config");
const database = require("../config/databaseParams");
const realmConfig = require("../config/realmConfiguration");
const { v4: uuidv4 } = require("uuid");

const app = new Realm.App({ id: config.realmAppId });
const credentials = Realm.Credentials.apiKey(config.realmApiKey);

const sendMessageToClient = (ws, message) => {
  ws.send(JSON.stringify(message));
};

/**
 * Creates a message header object with the specified type and properties from the given original messageHeader.
 *
 * @param {Object} messageHeader - The original message header object.
 * @param {string} type - The type of the message header, e.g., "update" or "read".
 * @returns {Object} The newly created message header object.
 */
const createMessageHeader = (messageHeader, type) => {
  const header = {
    type,
    tree: messageHeader.tree,
    id: messageHeader.id,
    uuid: messageHeader.uuid,
  };

  if (type === "update") {
    header.dateTime = new Date().toISOString();
  }

  return header;
};

/**
 * Creates a new message object with a header and response nodes.
 *
 * @param {string} message - The message content.
 * @param {Array} responseNodes - An array of response nodes.
 * @param {string} type - The type of the message, e.g., "update" or "read".
 * @returns {Object} The newly created message object.
 */
const createMessage = (message, responseNodes, type) => {
  const newMessage = createMessageHeader(message, type);
  if (responseNodes.length === 1) {
    newMessage.node = responseNodes[0];
  } else {
    newMessage.nodes = responseNodes;
  }
  return newMessage;
};

/**
 * Parses the response from a read event.
 *
 * @param {Object} message - The message object containing node or nodes information.
 * @param {Object} queryResponseObj - The query response object containing values to be mapped.
 * @returns {Object} - A data object with keys from the message nodes and values from the query response.
 */
function parseReadResponse(message, queryResponseObj) {
  const data = [];
  const nodes = message.node ? [message.node] : message.nodes;
  nodes.forEach((node) => {
    const prop = node.name;
    data.push({
      name: prop,
      value: queryResponseObj[prop],
    });
  });
  return data;
}

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
    this.sendMessageToClients = null;
  }

  async authenticateAndConnect(sendMessageToClients) {
    try {
      this.sendMessageToClients = sendMessageToClients;
      const user = await app.logIn(credentials);
      console.log("Successfully authenticated with Realm");

      const realmConfigObj = realmConfig(user);
      this.realm = await Realm.open(realmConfigObj);
      console.log("Realm connection established successfully");

      Object.entries(database).forEach(async ([key, value]) => {
        try {
          const databaseName = value.databaseName;
          await this.realm.objects(databaseName).subscribe();
          console.log(`Subscribed to the database ${key}: ${databaseName}`);
        } catch (error) {
          throw new Error("Error subscribing databases.");
        }
      });
    } catch (error) {
      console.error("Failed to authenticate with Realm:", error);
    }
  }

  async read(message, ws) {
    try {
      const updateMessage = await this.#getMessageData(message, ws);
      console.log(updateMessage);
      sendMessageToClient(ws, JSON.stringify(updateMessage));
    } catch (error) {
      console.error("Error reading object from Realm:", error);
      sendMessageToClient(
        ws,
        JSON.stringify({ error: "Error reading object" })
      );
    }
  }

  async write(message, ws) {
    try {
      const mediaElement = await this.#getMediaElement(message, ws);
      let nodes = message.node ? [message.node] : message.nodes;
      const endpoints = [];

      if (mediaElement) {
        this.realm.write(() => {
          nodes.forEach(({ name, value }) => {
            endpoints.push(name);
            mediaElement[name] = value;
          });
        });
      } else {
        const endpointId = database[message.tree].endpointId;
        let document = { _id: uuidv4(), [endpointId]: message.id };

        nodes.forEach(({ name, value }) => {
          endpoints.push(name);
          document[name] = value;
        });
        const databaseName = database[message.tree].databaseName;

        this.realm.write(() => {
          this.realm.create(databaseName, document);
        });
      }

      let messageNodes = endpoints.map((key) => ({ name: key }));
      const readMessage = createMessage(message, messageNodes, "read");
      const updateMessage = await this.#getMessageData(readMessage, ws);
      console.log(updateMessage);
      this.sendMessageToClients(JSON.stringify(updateMessage));
    } catch (error) {
      console.error("Error writing object changes in Realm:", error);
      sendMessageToClient(ws, { error: "Error writing object changes" });
    }
  }

  async subscribe(message, ws) {
    try {
      const mediaElement = await this.#getMediaElement(message, ws);
      if (mediaElement) {
        const objectId = mediaElement._id;
        const { id, tree, uuid } = message;
        const { databaseName, endpointId } = database[tree];
        console.log(
          `Subscribing element: Object ID: ${objectId} with ${endpointId}: '${id}' on ${databaseName}`
        );

        const mediaElementToSubscribe = await this.realm.objectForPrimaryKey(
          databaseName,
          objectId
        );

        if (mediaElementToSubscribe) {
          mediaElementToSubscribe.addListener(
            (mediaElementToSubscribe, changes) =>
              this.#onMediaElementChange(mediaElementToSubscribe, changes, {
                id: id,
                tree: tree,
                uuid: uuid,
              })
          );
          console.log(`Subscribed to changes for Object ID: ${objectId}`);
          sendMessageToClient(ws, {
            success: `Subscribed to changes for Object ID: ${objectId}`,
          });
        } else {
          console.log(`Object could not be subscribed`);
          sendMessageToClient(
            ws,
            JSON.stringify({ error: "Object could not be subscribed" })
          );
        }
      } else {
        console.log(`Object not found`);
        sendMessageToClient(ws, JSON.stringify({ error: "Object not found" }));
      }
    } catch (error) {
      console.error("Error subscribing to object changes in Realm:", error);
      sendMessageToClient(ws, { error: "Error subscribing to object changes" });
    }
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
    console.log("mediaElement: ", mediaElement);
    if (mediaElement) {
      const responseNodes = parseReadResponse(message, mediaElement);
      return createMessage(message, responseNodes, "update");
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
      const { databaseName, endpointId } = database[tree];
      return await this.realm
        .objects(databaseName)
        .filtered(`${endpointId} = '${id}'`)[0];
    } catch (error) {
      console.error("Error reading object from Realm:", error);
      sendMessageToClient(
        ws,
        JSON.stringify({ error: "Error reading object" })
      );
    }
  }

  /**
   * Handles changes to a media element and sends update messages to clients.
   * @param {mediaElement} mediaElement - The media element that has changed.
   * @param {object} changes - An object containing information about the changes.
   * @param {messageHeader} messageHeader - The header information for the message.
   */
  #onMediaElementChange(mediaElement, changes, messageHeader) {
    if (changes.deleted) {
      console.log(`MediaElement is deleted: ${changes.deleted}`);
    } else {
      if (changes.changedProperties.length > 0) {
        const responseNodes = parseOnMediaElementChangeResponse(
          changes,
          mediaElement
        );
        const updateMessage = createMessage(
          messageHeader,
          responseNodes,
          "update"
        );
        console.log(updateMessage);
        this.sendMessageToClients(JSON.stringify(updateMessage));
      }
    }
  }
}

module.exports = RealmDBHandler;
