import { getEnvValue } from "../../../config/config";
import { DatabaseParamsRecord } from "../../../../utils/database-params";

/**
 * Defines the shape of the IoTDB configuration object.
 */
interface IotDBConfig {
  iotdbHost: string;
  iotdbPort: number;
  iotdbUser: string;
  iotdbPassword: string;
  fetchSize: number;
  timeZoneId: string;
  pollIntervalLenInSec: number;
}

/*
 * Contains the definition of the database name and its identifier data point for each catalog.
 */
export const databaseParams: Readonly<DatabaseParamsRecord> = {
  VSS: {
    databaseName: "root.Vehicles", // name of the configured IoTDB for the VSS database
    dataPointId: "Vehicle_VehicleIdentification_VIN", // data point used as element ID
  },
};

/**
 * Retrieves the database configuration for IoTDB.
 *
 * This function gathers configuration values for connecting to an IoTDB instance.
 * It first attempts to get the values from environment variables. If the environment
 * variables are not set, it uses default values.
 *
 * @returns {IotDBConfig} An object containing the IoTDB configuration.
 */
const getDatabaseConfig = (): Readonly<IotDBConfig> => {
  const iotdb_config: IotDBConfig = {
    iotdbHost: getEnvValue("IOTDB_HOST") || "iotdb-service",
    iotdbPort: Number(getEnvValue("IOTDB_PORT")) || 6667,
    iotdbUser: getEnvValue("IOTDB_USER") || "root",
    iotdbPassword: getEnvValue("IOTDB_PASSWORD") || "root",
    fetchSize: Number(getEnvValue("IOTDB_FETCH_SIZE")) || 10000,
    timeZoneId:
      getEnvValue("IOTDB_TIME_ZONE_ID") ||
      Intl.DateTimeFormat().resolvedOptions().timeZone,
    pollIntervalLenInSec: Number(getEnvValue("IOTDB_POLL_INTERVAL_LEN_IN_SEC")) || 0.2,   
  };

  return iotdb_config;
};

/**
 * If the HANDLER_TYPE is set to 'iotdb', the IoTDB configuration is used.
 */
export const databaseConfig: Readonly<IotDBConfig> | undefined =
  getEnvValue("HANDLER_TYPE") === "iotdb" ? getDatabaseConfig() : undefined;
