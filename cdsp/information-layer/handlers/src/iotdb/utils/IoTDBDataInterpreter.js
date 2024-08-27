const {
  IoTDBDataType,
  SupportedMessageDataTypes,
} = require("./IoTDBConstants");

class IoTDBDataInterpreter {
  /**
   * Serializes values based on the specified data types.
   * @param {Array} dataTypes - Array of data types to serialize the values as.
   * @param {Array} values - Array of values to be serialized.
   * @returns {Buffer} - Serialized values as a Buffer.
   */
  static serializeValues(dataTypes, values) {
    function serializeBoolean(value) {
      return [IoTDBDataType.BOOLEAN, value];
    }

    function serializeInt32(value) {
      const int32 = new Int32Array([value]);
      const uint8 = new Uint8Array(int32.buffer).reverse();
      return [IoTDBDataType.INT32, ...uint8];
    }

    function serializeInt64(value) {
      const bigint64 = new BigInt64Array([value]);
      const uint8 = new Uint8Array(bigint64.buffer).reverse();
      return [IoTDBDataType.INT64, ...uint8];
    }

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
        case SupportedMessageDataTypes.boolean:
          serializedValues.push(...serializeBoolean(values[i]));
          break;
        case SupportedMessageDataTypes.int8:
        case SupportedMessageDataTypes.int16:
        case SupportedMessageDataTypes.uint8:
        case SupportedMessageDataTypes.uint16:
          serializedValues.push(...serializeInt32(values[i]));
          break;
        // case SupportedMessageDataTypes.int64: // this type is not supported by now, see: cdsp/information-layer/handlers/iotdb/utils/IoTDBConstants.js
        //   serializedValues.push(...serializeInt64(values[i]));
        //   break;
        case SupportedMessageDataTypes.float:
          serializedValues.push(...serializeFloat(values[i]));
          break;
        case SupportedMessageDataTypes.double:
          serializedValues.push(...serializeDouble(values[i]));
          break;
        case SupportedMessageDataTypes.string:
          serializedValues.push(...serializeText(values[i]));
          break;
        default:
          throw new Error(`Unsupported data type: ${dataTypes[i]}`);
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
        const newKey = key.replace(`${databaseName}.`, "");
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
}

module.exports = { IoTDBDataInterpreter };
