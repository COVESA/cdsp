const { IoTDBDataType, MessageDataType } = require("./IoTDBConstants");
const database = require("../config/databaseParams");

class IoTDBDataInterpreter {
  /**
   * Serializes values based on the specified data types.
   * @param {Array} dataTypes - Array of data types to serialize the values as.
   * @param {Array} values - Array of values to be serialized.
   * @returns {Buffer} - Serialized values as a Buffer.
   */
  static serializeValues(dataTypes, values) {
    // this type is not supported by now, see: cdsp/information-layer/handlers/iotdb/utils/IoTDBConstants.js
    // function serializeBoolean(value) {
    //   return [IoTDBDataType.BOOLEAN, value];
    // }

    function serializeInt32(value) {
      const int32 = new Int32Array([value]);
      const uint8 = new Uint8Array(int32.buffer).reverse();
      return [IoTDBDataType.INT32, ...uint8];
    }

    // this type is not supported by now, see: cdsp/information-layer/handlers/iotdb/utils/IoTDBConstants.js
    // function serializeInt64(value) {
    //   const bigint64 = new BigInt64Array([value]);
    //   const uint8 = new Uint8Array(bigint64.buffer).reverse();
    //   return [IoTDBDataType.INT64, ...uint8];
    // }

    function serializeFloat(value) {
      const float32 = new Float32Array([value]);
      const uint8 = new Uint8Array(float32.buffer).reverse();
      return [IoTDBDataType.FLOAT, ...uint8];
    }

    function serializeDouble(value) {
      const float64 = new Float64Array([value]);
      const uint8 = new Uint8Array(float64.buffer).reverse();
      return [IoTDBDataType.DOUBLE, ...uint8];
    }

    function serializeText(value) {
      const utf8arr = Buffer.from(value);
      const int32 = new Uint32Array([utf8arr.length]);
      const uint8 = new Uint8Array(int32.buffer).reverse();
      return [IoTDBDataType.TEXT, ...uint8, ...utf8arr];
    }

    const serializedValues = [];

    for (let i = 0; i < dataTypes.length; i++) {
      switch (dataTypes[i]) {
        // case MessageDataType.BOOLEAN: // this type is not supported by now, see: cdsp/information-layer/handlers/iotdb/utils/IoTDBConstants.js
        //   serializedValues.push(...serializeBoolean(values[i]));
        //   break;
        case MessageDataType.INT8:
        case MessageDataType.INT16:
        case MessageDataType.UINT16:
          //case MessageDataType.UINT32:
          serializedValues.push(...serializeInt32(values[i]));
          break;
        // case MessageDataType.INT64: // this type is not supported by now, see: cdsp/information-layer/handlers/iotdb/utils/IoTDBConstants.js
        //   serializedValues.push(...serializeInt64(values[i]));
        //   break;
        case MessageDataType.FLOAT:
          serializedValues.push(...serializeFloat(values[i]));
          break;
        case MessageDataType.DOUBLE:
          serializedValues.push(...serializeDouble(values[i]));
          break;
        case MessageDataType.STRING:
          serializedValues.push(...serializeText(values[i]));
          break;
        default:
          throw new Error("Unsupported data type");
      }
    }
    return Buffer.from(serializedValues);
  }

  /**
   * Extracts and transforms timeseries nodes from the given object with the required message format.
   *
   * @param {Object} obj - The object containing timeseries data.
   * @param {string} databaseName - The database name to match and remove from the keys.
   * @returns {Object} - A new object with transformed keys and their corresponding values.
   */
  static extractNodesFromTimeseries(obj, databaseName) {
    return Object.entries(obj).reduce((acc, [key, value]) => {
      if (key.startsWith(databaseName)) {
        const newKey = key.replace(`${databaseName}.`, "").replace(/_/g, ".");
        acc[newKey] = value;
      }
      return acc;
    }, {});
  }

  /**
   * This function converts a buffer to a BigInt64Array and extracts the timestamp.
   *
   * @param {Buffer} buffer - The buffer containing the timestamp.
   * @returns {{ timestamp: BigInt }} An object containing the extracted timestamp.
   */
  static extractTimestamp(buffer) {
    const reverseBuffer = Buffer.from(buffer.subarray(0, 8).reverse());
    const uinit8Buffer = new Uint8Array(reverseBuffer).buffer;
    const timestamp = new BigInt64Array(uinit8Buffer)[0];

    return { timestamp };
  }

  /**
   * Extracts endpoint names from the given message.
   *
   * This function checks if the message has a single node or multiple nodes and
   * extracts the names accordingly.
   *
   * @param {Object} message - The message containing node(s).
   * @returns {Array<string>} An array of endpoint names.
   */
  static extractEndpointsFromNodes(message) {
    let endpoints = [];

    if (message.node) {
      endpoints.push(this.transformEndpointFromMessageNode(message.node.name));
    } else if (message.nodes) {
      endpoints = message.nodes.map((node) =>
        this.transformEndpointFromMessageNode(node.name)
      );
    }
    return endpoints;
  }

  /**
   * Transforms a message node by replacing all dots with underscores.
   *
   * @param {string} node - The message node to transform.
   * @returns {string} - The transformed message node with dots replaced by underscores.
   */
  static transformEndpointFromMessageNode(node) {
    return `${node}`.replace(/\./g, "_");
  }
}

module.exports = { IoTDBDataInterpreter };
