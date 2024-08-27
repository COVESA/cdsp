const { SupportedMessageDataTypes } = require("../utils/IoTDBConstants");

class EndpointsSchema {
  constructor(supportedEndpoints) {
    const properties = {};

    Object.entries(supportedEndpoints).forEach(([key, value]) => {
      if (SupportedMessageDataTypes.hasOwnProperty(value)) {
        properties[key] = value;
      } else {
        throw new Error(
          `The initialized endpoints contains an unsupported data type: ${value}`
        );
      }
    });

    this.endpointsSchema = properties;
    Object.freeze(this.endpointsSchema);
  }

  getEndpointsSchema() {
    return this.endpointsSchema;
  }
}

// Singleton instance holder
let endpointsSchemaInstance = null;

/**
 * Creates and returns a singleton instance of EndpointsSchema.
 * If the instance does not already exist, it initializes it with the provided supported endpoints
 * and freezes the instance to prevent further modifications.
 *
 * @param {Object} supportedEndpoints - An object of supported endpoints to initialize the schema.
 * @returns {Object} The endpoints schema instance.
 */
function createEndpointsSchema(supportedEndpoints) {
  if (!endpointsSchemaInstance) {
    endpointsSchemaInstance = new EndpointsSchema(supportedEndpoints);
    Object.freeze(endpointsSchemaInstance);
  }

  return endpointsSchemaInstance.getEndpointsSchema();
}

module.exports = { createEndpointsSchema };
