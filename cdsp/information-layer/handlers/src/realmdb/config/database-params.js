const { getEnvValue } = require("../../../config/config");

/*
 * Contains the definition of the database name and its identifier data point for each catalog.
 */
const mediaElementsParams = Object.freeze({
  VSS: {
    databaseName: "Vehicles", // name of the configured RealmDB for the VSS database
    dataPointId: "Vehicle_VehicleIdentification_VIN", // data point used as element ID
  },
});

/**
 * Retrieves the database configuration.
 *
 * This function fetches the Realm application ID and API key from the environment variables.
 * If either of these values is not found, it throws an error.
 *
 * @throws {Error} If the REALMDB_APP_ID is not set in the environment variables.
 * @throws {Error} If the REALMDB_API_KEY is not set in the environment variables.
 *
 * @returns {Object} The database configuration object containing:
 * - storePath: The path to the Realm database file.
 * - realmAppId: The Realm application ID.
 * - realmApiKey: The Realm API key.
 */
const getDatabaseConfig = () => {
  const realmAppId = getEnvValue("REALMDB_APP_ID");
  const realmApiKey = getEnvValue("REALMDB_API_KEY");
  if (!realmAppId) {
    throw new Error("REALMDB_APP_ID is required, any default has been set");
  } else if (!realmApiKey) {
    throw new Error("REALMDB_API_KEY is required, any default has been set");
  }

  return Object.freeze({
    storePath: "myrealm12.realm",
    realmAppId: realmAppId,
    realmApiKey: realmApiKey,
  });
};

const databaseConfig =
  getEnvValue("HANDLER_TYPE") === "realmdb" ? getDatabaseConfig() : {};

module.exports = { mediaElementsParams, databaseConfig };
