import { getEnvValue } from "../../../config/config";
import { DatabaseParamsRecord } from "../../../../utils/database-params";

interface RealmDBConfig {
  storePath: string;
  realmAppId: string;
  realmApiKey: string;
}

/**
 * Contains the definition of the database name and its identifier data point for each catalog.
 */
export const mediaElementsParams: Readonly<DatabaseParamsRecord> = {
  VSS: {
    databaseName: "Vehicles", // name of the configured RealmDB for the VSS database
    dataPointId: "Vehicle_VehicleIdentification_VIN", // data point used as element ID
  },
};

/**
 * Retrieves the database configuration as a read-only object.
 *
 * This function fetches the Realm application ID and API key from the environment variables.
 * It ensures that both values are present, throwing an error if either is missing.
 *
 * @returns The database configuration object containing the store path, Realm app ID, and API key.
 *
 * @throws If the REALMDB_APP_ID or REALMDB_API_KEY environment variables are not set.
 */
const getDatabaseConfig = (): Readonly<RealmDBConfig> => {
  const realmAppId = getEnvValue("REALMDB_APP_ID");
  const realmApiKey = getEnvValue("REALMDB_API_KEY");

  if (!realmAppId) {
    throw new Error("REALMDB_APP_ID is required, but no default has been set");
  } else if (!realmApiKey) {
    throw new Error("REALMDB_API_KEY is required, but no default has been set");
  }

  return {
    storePath: "myrealm12.realm",
    realmAppId,
    realmApiKey,
  };
};

/**
 * Exports the database configuration depending on the handler type.
 */
export const databaseConfig =
  getEnvValue("HANDLER_TYPE") === "realmdb" ? getDatabaseConfig() : undefined;

/**
 * Primary Key that should be used when storing entities in RealmDB
 */
export const PRIMARY_KEY = "_id"
