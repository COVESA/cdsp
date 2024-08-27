const config = require("../config/config");
const fs = require("fs");
const yaml = require("js-yaml");
const { logMessage, MessageType } = require("../../utils/logger");
/**
 * Reads and parses an endpoints file in either JSON, YML or YAML format.
 *
 * @param {string} filePath - The path to the endpoints file.
 * @returns {Object} The parsed content of the endpoints file.
 * @throws {Error} If the file format is unsupported.
 */
function readEndpointsFile(filePath) {
  const fileContent = fs.readFileSync(filePath, "utf8");
  if (filePath.endsWith(".json")) {
    return JSON.parse(fileContent);
  } else if (filePath.endsWith(".yaml") || filePath.endsWith(".yml")) {
    return yaml.load(fileContent);
  } else {
    throw new Error("Unsupported endpoints file format");
  }
}

/**
 * Recursively extracts data types from an endpoints object.
 *
 * @param {Object} endpointsObj - The object containing endpoint definitions.
 * @param {string} [parentKey=""] - The parent key used to build the nested key path.
 * @param {Object} [result={}] - The object to store the extracted data types.
 * @returns {Object} An object mapping endpoint keys to their data types.
 */
function extractDataTypes(endpointsObj, parentKey = "", result = {}) {
  for (const key in endpointsObj) {
    if (endpointsObj.hasOwnProperty(key)) {
      const value = endpointsObj[key];
      const newKey = parentKey ? `${parentKey}.${key}` : key;
      if (value && typeof value === "object") {
        if (value.datatype) {
          result[newKey] = value.datatype;
        } else {
          extractDataTypes(value.children || value, newKey, result);
        }
      }
    }
  }
  return result;
}

class Handler {
  handleMessage(message, ws) {
    try {
      switch (message.type) {
        case "read":
          this.read(message, ws);
          break;
        case "write":
          this.write(message, ws);
          break;
        case "subscribe":
          this.subscribe(message, ws);
          break;
        default:
          ws.send(JSON.stringify({ error: "Unknown message type" }));
      }
    } catch (error) {
      ws.send(JSON.stringify({ error: error.message }));
    }
  }

  authenticateAndConnect(sendMessageToClients) {
    throw new Error("Method 'authenticateAndConnect' must be implemented.");
  }

  read(message, ws) {
    throw new Error("Read method not implemented yet!");
  }

  write(message, ws) {
    throw new Error("Write method not implemented yet!");
  }

  subscribe(message, ws) {
    throw new Error("Subscribe method not implemented yet!");
  }

  /**
   * Utility functions for handling messages and data structures common to IoTDB and RealmDB.
   */

  /**
   * Sends a message to the client
   * @param {WebSocket} ws - The WebSocket connection to send the response to.
   * @param {Object} message - The message to be sent to the client.
   */
  _sendMessageToClient = (ws, message) => {
    logMessage(JSON.stringify(message), MessageType.SENT);
    ws.send(JSON.stringify(message));
  };

  /**
   * Generic function to create an update message.
   * @param {Object} message - The original message from client.
   * @param {Array} nodes - The nodes to be included in the message.
   * @returns {Object} - The transformed message.
   */
  _createUpdateMessage(message, nodes) {
    const { id, tree, uuid } = message;
    let newMessage = {
      type: "update",
      tree,
      id,
      dateTime: new Date().toISOString(),
      uuid,
    };
    if (nodes.length === 1) {
      newMessage["node"] = nodes[0];
    } else {
      newMessage["nodes"] = nodes;
    }
    return newMessage;
  }

  /**
   * Transforms a message node by replacing all dots with underscores.
   *
   * @param {string} node - The message node to transform.
   * @returns {string} - The transformed message node with dots replaced by underscores.
   */
  _transformEndpointsWithUnderscores(node) {
    return `${node}`.replace(/\./g, "_");
  }

  /**
   * Transforms a database field name by replacing underscores with all dots.
   *
   * @param {string} field - The database filed to transform.
   * @returns {string} - The transformed to message node replacing underscores by dots.
   */
  _transformEndpointsWithDots(field) {
    return `${field}`.replace(/\_/g, ".");
  }

  /**
   * Retrieves and processes supported endpoints.
   *
   * This method reads the endpoints configuration file, extracts the data types,
   * and transforms the endpoint names to use underscores. It returns an object
   * with the transformed endpoint names as keys and their corresponding data types.
   *
   * @returns {Object} An object containing the supported endpoints with transformed names and data types.
   */
  _getSupportedEndpoints() {
    const endpointPath = config.getEndpointsPath();
    const endpointObj = readEndpointsFile(endpointPath);
    const supportedEndpoints = extractDataTypes(endpointObj);
    let result = {};
    Object.entries(supportedEndpoints).forEach(([node, value]) => {
      const underscored_node = this._transformEndpointsWithUnderscores(node);
      if (value !== null) {
        result[underscored_node] = value;
      }
    });
    return result;
  }
}

module.exports = Handler;
