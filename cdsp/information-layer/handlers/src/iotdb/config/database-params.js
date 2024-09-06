const { getEnvValue } = require("../../../config/config");

/*
 * Contains the definition of the database name and its identifier data point for each catalog.
 */
const databaseParams = Object.freeze({
  VSS: {
    databaseName: "root.Vehicles", // name of the configured IoTDB for the VSS database
    dataPointId: "Vehicle_VehicleIdentification_VIN", // data point used as element ID
  },
});

const useDefaultValue = (defaultValue) => {
  console.info(`Using default: ${defaultValue}`);
  return defaultValue;
};

/**
 * Retrieves the database configuration for IoTDB.
 *
 * This function gathers configuration values for connecting to an IoTDB instance.
 * It first attempts to get the values from environment variables. If the environment
 * variables are not set, it uses default values.
 *
 * @returns {Object} An object containing the IoTDB configuration.
 */
const getDatabaseConfig = () => {
  const iotdb_config = {};

  let default_timeZoneId = Intl.DateTimeFormat().resolvedOptions().timeZone;

  iotdb_config["iotdbHost"] =
    getEnvValue("IOTDB_HOST") || useDefaultValue("iotdb-service");
  iotdb_config["iotdbPort"] =
    getEnvValue("IOTDB_PORT") || useDefaultValue(6667);
  iotdb_config["iotdbUser"] =
    getEnvValue("IOTDB_USER") || useDefaultValue("root");
  iotdb_config["iotdbPassword"] =
    getEnvValue("IOTDB_PASSWORD") || useDefaultValue("root");
  iotdb_config["fetchSize"] =
    getEnvValue("IOTDB_FETCH_SIZE") || useDefaultValue(10000);
  iotdb_config["timeZoneId"] =
    getEnvValue("IOTDB_TIME_ZONE_ID") || useDefaultValue(default_timeZoneId);

  return Object.freeze(iotdb_config);
};

const databaseConfig =
  getEnvValue("HANDLER_TYPE") === "iotdb" ? getDatabaseConfig() : {};

module.exports = { databaseParams, databaseConfig };
