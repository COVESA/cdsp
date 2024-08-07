const Realm = require("realm");
const Handler = require("../../handler");
const config = require("../config/config");
const Database = require("../config/Database");
const realmConfig = require("../config/RealmConfiguration");

const app = new Realm.App({ id: config.realmAppId });
const credentials = Realm.Credentials.apiKey(config.realmApiKey);

const sendMessageToClient = (ws, message) => {
  ws.send(JSON.stringify(message));
};

/**
 * Creates the update message header object with the required information.
 *
 * @param {Object} messageHeader - The original message header object.
 * @returns {Object} The updated message header object.
 */
const UpdateMessageHeader = (messageHeader) => ({
  type: "update",
  tree: messageHeader.tree,
  id: messageHeader.id,
  dateTime: new Date().toISOString(),
  uuid: messageHeader.uuid,
});

/**
 * Parses the response from a read event.
 *
 * @param {Object} message - The message object containing node or nodes information.
 * @param {Object} queryResponseObj - The query response object containing values to be mapped.
 * @returns {Object} - A data object with keys from the message nodes and values from the query response.
 */
function parseReadResponse(message, queryResponseObj) {
  const data = [];
  if (message.node) {
    const prop = message.node.name;
    data.push({
      name: prop,
      value: queryResponseObj[prop],
    });
  } else if (message.nodes) {
    message.nodes.forEach((node) => {
      const prop = node.name;
      data.push({
        name: prop,
        value: queryResponseObj[prop],
      });
    });
  }
  return data;
}

/**
 * Parses the response from a media element change event.
 *
 * @param {Object} changes - The object containing the changed properties.
 * @param {Object} MediaElement - The media element object with updated properties.
 * @returns {Array} An array of objects, each containing the name and value of a changed property.
 */
function parseOnMediaElementChangeResponse(changes, MediaElement) {
  const data = [];
  changes.changedProperties.forEach((prop) => {
    data.push({
      name: prop,
      value: MediaElement[prop],
    });
  });
  return data;
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

      // const MediaElements = await this.realm.objects("Vehicles").subscribe(); // TODO: Is it necessary to log this, extra function?
      // if (MediaElements) {
      //   console.log(MediaElements);
      // } else {
      //   this.sendMessageToClients({
      //     error: "Vehicles collection not found for subscription",
      //   });
      // }
    } catch (error) {
      console.error("Failed to authenticate with Realm:", error);
    }
  }

  async read(message, ws) {
    try {
      const idValue = message.id;
      const databaseName = Database[message.tree].database_name;
      const idEndpoint = Database[message.tree].endpoint_id;
      const mediaElement = await this.#getMediaElement(
        databaseName,
        idEndpoint,
        idValue,
        ws
      );
      const responseNodes = parseReadResponse(message, mediaElement);
      let updateMessage = UpdateMessageHeader(message);
      if (responseNodes.length == 1) {
        updateMessage["node"] = responseNodes[0];
      } else {
        updateMessage["nodes"] = responseNodes;
      }

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

  async subscribe(message, ws) {
    try {
      const idValue = message.id;
      const databaseName = Database[message.tree].database_name;
      const idEndpoint = Database[message.tree].endpoint_id;
      const MediaElement = await this.#getMediaElement(
        databaseName,
        idEndpoint,
        idValue,
        ws
      );

      if (MediaElement) {
        const objectId = MediaElement._id;

        console.log(
          `Subscribing element: Object ID: ${objectId} with ${idEndpoint}: '${idValue}' on ${databaseName}`
        );
        const MediaElementToSubscribe = await this.realm.objectForPrimaryKey(
          databaseName,
          objectId
        );

        if (MediaElementToSubscribe) {
          MediaElementToSubscribe.addListener(
            (MediaElementToSubscribe, changes) =>
              this.#onMediaElementChange(MediaElementToSubscribe, changes, {
                id: idValue,
                tree: message.tree,
                uuid: message.uuid,
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
   * Asynchronously retrieves a media element from a Realm database based on the provided criteria.
   * @param {string} databaseName - The name of the Realm database to query.
   * @param {string} idEndpoint - The field in the database to filter on.
   * @param {string} idValue - The value to match in the specified field.
   * @param {WebSocket} ws - The WebSocket connection to send messages to the client.
   * @returns {Promise<object>} A Promise that resolves to the retrieved media element or rejects with an error.
   */
  async #getMediaElement(databaseName, idEndpoint, idValue, ws) {
    try {
      return await this.realm
        .objects(databaseName)
        .filtered(`${idEndpoint} = '${idValue}'`)[0];
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
   * @param {MediaElement} MediaElement - The media element that has changed.
   * @param {object} changes - An object containing information about the changes.
   * @param {MessageHeader} MessageHeader - The header information for the message.
   */
  #onMediaElementChange(MediaElement, changes, MessageHeader) {
    if (changes.deleted) {
      console.log(`MediaElement is deleted: ${changes.deleted}`);
    } else {
      if (changes.changedProperties.length > 0) {
        const responseNodes = parseOnMediaElementChangeResponse(
          changes,
          MediaElement
        );
        let updateMessage = UpdateMessageHeader(MessageHeader);
        if (responseNodes.length == 1) {
          updateMessage["node"] = responseNodes[0];
        } else {
          updateMessage["nodes"] = responseNodes;
        }

        console.log(updateMessage);
        this.sendMessageToClients(JSON.stringify(updateMessage));
      }
    }
  }
}

module.exports = RealmDBHandler;
