const config = require("../config/config");
const fs = require("fs");
const yaml = require("js-yaml");
const { logMessage, MessageType } = require("../../utils/logger");
/**
 * Reads and parses a data points file in either JSON, YML or YAML format.
 *
 * @param {string} filePath - The path to the data points file.
 * @returns {Object} The parsed content of the data points file.
 * @throws {Error} If the file format is unsupported.
 */
function readDataPointsFile(filePath) {
  const fileContent = fs.readFileSync(filePath, "utf8");
  if (filePath.endsWith(".json")) {
    return JSON.parse(fileContent);
  } else if (filePath.endsWith(".yaml") || filePath.endsWith(".yml")) {
    return yaml.load(fileContent);
  } else {
    throw new Error("Unsupported data points file format");
  }
}

/**
 * Recursively extracts data types from a data point object.
 *
 * @param {Object} dataPointsObj - The object containing data point definitions.
 * @param {string} [parentKey=""] - The parent key used to build the nested key path.
 * @param {Object} [result={}] - The object to store the extracted data types.
 * @returns {Object} An object mapping data point keys to their data types.
 */
function extractDataTypes(dataPointsObj, parentKey = "", result = {}) {
  for (const key in dataPointsObj) {
    if (dataPointsObj.hasOwnProperty(key)) {
      const value = dataPointsObj[key];
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
        case "unsubscribe":
          this.unsubscribe(message, ws);
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

  unsubscribe(message, ws) {
    throw new Error("Unsubscribe method not implemented yet!");
  }

  unsubscribe_client(uuid) {
    throw new Error("Unsubscribe client method not implemented yet!");
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
   * Generic function to create or remove a subscription message.
   * @param {{"subscribe", "unsubscribe"}} type - Type of subscription message.
   * @param {Object} message - The original message from client.
   * @param {String} status - The status of the subscription.
   * @returns {Object} - The transformed message.
   */
  _createSubscribeMessage(type, message, status) {
    const { id, tree, uuid } = message;
    return {
      type: `${type}:status`,
      tree,
      id,
      dateTime: new Date().toISOString(),
      uuid,
      status: status,
    };
  }

  /**
   * Transforms a message node by replacing all dots with underscores.
   *
   * @param {string} node - The message node to transform.
   * @returns {string} - The transformed message node with dots replaced by underscores.
   */
  _transformDataPointsWithUnderscores(node) {
    return `${node}`.replace(/\./g, "_");
  }

  /**
   * Transforms a database field name by replacing underscores with all dots.
   *
   * @param {string} field - The database filed to transform.
   * @returns {string} - The transformed to message node replacing underscores by dots.
   */
  _transformDataPointsWithDots(field) {
    return `${field}`.replace(/\_/g, ".");
  }

  /**
   * Retrieves and processes supported data points.
   *
   * This method reads the data points configuration file, extracts the data types,
   * and transforms the data point names to use underscores. It returns an object
   * with the transformed data point names as keys and their corresponding data types.
   *
   * @returns {Object} An object containing the supported data points with transformed names and data types.
   */
  _getSupportedDataPoints() {
    const datapointPath = config.getDataPointsPath();
    const dataPointObj = readDataPointsFile(datapointPath);
    const supportedDataPoints = extractDataTypes(dataPointObj);
    let result = {};
    Object.entries(supportedDataPoints).forEach(([node, value]) => {
      const underscored_node = this._transformDataPointsWithUnderscores(node);
      if (value !== null) {
        result[underscored_node] = value;
      }
    });
    return result;
  }

  /**
   * Validates nodes against a given schema.
   *
   * @param {Object} message - The message containing nodes to be validated.
   * @param {Object} dataPointsSchema - The schema against which nodes are validated.
   * @returns {Object|null} An object containing error details if validation fails, otherwise null.
   */
  _validateNodesAgainstSchema(message, dataPointsSchema) {
    const { type } = message;
    let nodes = message.node ? [message.node] : message.nodes;

    // Check if all nodes are valid based on the schema
    const unknownFields = nodes.filter(({ name }) => {
      const transformedName = this._transformDataPointsWithUnderscores(name);
      return !dataPointsSchema.hasOwnProperty(transformedName);
    });

    if (unknownFields.length > 0) {
      let errors = unknownFields.map(({ name }) => ({
        name: name,
        status: "Parent object or node not found.",
      }));

      const errorData =
        errors.length === 1 ? { node: errors.at(0) } : { nodes: errors };

      return errorData;
    }
    return null;
  }
}

module.exports = Handler;
