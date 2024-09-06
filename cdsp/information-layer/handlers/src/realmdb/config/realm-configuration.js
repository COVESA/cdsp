const { mediaElementsParams, databaseConfig } = require("./database-params");
const { getEnvValue } = require("../../../config/config");

/**
 * Configures the Realm database settings.
 *
 * @param {Object} user - The user object for authentication.
 * @param {Array} supportedEndpoints - List of supported endpoints for the media element schema.
 * @returns {Object} - The configuration object for the Realm database.
 */
function realmConfig(user, supportedDataPoints) {
  const mediaElementSchema = createMediaElementSchema(supportedDataPoints);
  return {
    schema: [mediaElementSchema],
    path: databaseConfig.storePath,
    sync: {
      user: user,
      flexible: true,
      error: (error) => {
        console.error("Realm sync error:", error);
      },
    },
    // `schemaVersion` is by default 0, it can be incremented when the schema changes,
    // but you need to write the logic to migrate existing data to the new schema.
    // See: https://www.mongodb.com/docs/atlas/device-sdks/sdk/node/model-data/modify-an-object-schema/
    schemaVersion: getSchemaVersion(),
    migration: (oldRealm, newRealm) => {
      // Perform migration logic here
    },
  };
}

const getSchemaVersion = () => {
  const schemaVersion = parseInt(getEnvValue("VERSION_REALMDB_SCHEMA"), 10);
  if (isNaN(schemaVersion)) {
    throw new Error(
      "Version must be specified as an ENV variable and it must be 0 or a positive integer",
    );
  }
  return schemaVersion;
};

function createMediaElementSchema(supportedEndpoints) {
  const properties = { _id: "string" };

  Object.entries(supportedEndpoints).forEach(([key, value]) => {
    switch (value) {
      case "boolean":
        properties[key] = "bool";
        break;
      case "string":
      case "float":
      case "double":
        properties[key] = value;
        break;
      case "int8":
      case "int16":
      case "uint8":
      case "uint16":
        properties[key] = "int";
        break;
      default:
        throw new Error(
          `The initialized data points contains an unsupported data type: ${value}`,
        );
    }
  });

  const mediaElementSchema = {
    primaryKey: "_id",
    name: mediaElementsParams.VSS.databaseName,
    properties: properties,
  };

  return mediaElementSchema;
}

module.exports = realmConfig;
