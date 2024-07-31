const JSDataType = require("./IoTDBConstants");

class IoTDBDataInterpreter {
  /**
   * Serializes values based on the specified data types.
   * @param {Array} dataTypes - Array of data types to serialize the values as.
   * @param {Array} values - Array of values to be serialized.
   * @returns {Buffer} - Serialized values as a Buffer.
   */
  static serializeValues(dataTypes, values) {
    function serializeBoolean(value) {
      return [JSDataType.BOOLEAN, value];
    }

    function serializeInt32(value) {
      const int32 = new Int32Array([value]);
      const uint8 = new Uint8Array(int32.buffer).reverse();
      return [JSDataType.INT32, ...uint8];
    }

    function serializeInt64(value) {
      const bigint64 = new BigInt64Array([value]);
      const uint8 = new Uint8Array(bigint64.buffer).reverse();
      return [JSDataType.INT64, ...uint8];
    }

    function serializeFloat(value) {
      const float32 = new Float32Array([value]);
      const uint8 = new Uint8Array(float32.buffer).reverse();
      return [JSDataType.FLOAT, ...uint8];
    }

    function serializeDouble(value) {
      const float64 = new Float64Array([value]);
      const uint8 = new Uint8Array(float64.buffer).reverse();
      return [JSDataType.DOUBLE, ...uint8];
    }

    function serializeText(value) {
      const utf8arr = Buffer.from(value);
      const int32 = new Uint32Array([utf8arr.length]);
      const uint8 = new Uint8Array(int32.buffer).reverse();
      return [JSDataType.TEXT, ...uint8, ...utf8arr];
    }

    const serializedValues = [];

    for (let i = 0; i < dataTypes.length; i++) {
      switch (dataTypes[i]) {
        case JSDataType.BOOLEAN:
          serializedValues.push(...serializeBoolean(values[i]));
          break;
        case JSDataType.INT32:
          serializedValues.push(...serializeInt32(values[i]));
          break;
        case JSDataType.INT64:
          serializedValues.push(...serializeInt64(values[i]));
          break;
        case JSDataType.FLOAT:
          serializedValues.push(...serializeFloat(values[i]));
          break;
        case JSDataType.DOUBLE:
          serializedValues.push(...serializeDouble(values[i]));
          break;
        case JSDataType.TEXT:
          serializedValues.push(...serializeText(values[i]));
          break;
        default:
          throw new Error("Unsupported data type");
      }
    }
    return Buffer.from(serializedValues);
  }

  /**
   * Transforms an object by extracting and renaming keys that start with device id 'root.Vehicles.'.
   *
   * @param {Object} obj - The input object to be transformed.
   * @returns {Object} - A new object with keys that were prefixed with 'root.Vehicles.' renamed to their last segment.
   */
  static extractDeviceIdFromTimeseries(obj) {
    return Object.entries(obj).reduce((acc, [key, value]) => {
      if (key.startsWith("root.Vehicles.")) {
        const newKey = key.split(".").pop(); // Get the last part of the key
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
