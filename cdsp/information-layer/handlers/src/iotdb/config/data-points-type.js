const { SupportedMessageDataTypes } = require("../utils/iotdb-constants");

class DataPointsSchema {
  constructor(supportedDataPoints) {
    const properties = {};

    Object.entries(supportedDataPoints).forEach(([key, value]) => {
      if (SupportedMessageDataTypes.hasOwnProperty(value)) {
        properties[key] = value;
      } else {
        throw new Error(
          `The initialized data points contains an unsupported data type: ${value}`,
        );
      }
    });

    this.dataPointsSchema = properties;
    Object.freeze(this.dataPointsSchema);
  }

  getDataPointsSchema() {
    return this.dataPointsSchema;
  }
}

// Singleton instance holder
let dataPointsSchemaInstance = null;

/**
 * Creates and returns a singleton instance of DataPointsSchema.
 * If the instance does not already exist, it initializes it with the provided supported data points
 * and freezes the instance to prevent further modifications.
 *
 * @param {Object} supportedEndpoints - An object of supported data points to initialize the schema.
 * @returns {Object} The data points schema instance.
 */
function createDataPointsSchema(supportedEndpoints) {
  if (!dataPointsSchemaInstance) {
    dataPointsSchemaInstance = new DataPointsSchema(supportedEndpoints);
    Object.freeze(dataPointsSchemaInstance);
  }

  return dataPointsSchemaInstance.getDataPointsSchema();
}

module.exports = { createDataPointsSchema };
