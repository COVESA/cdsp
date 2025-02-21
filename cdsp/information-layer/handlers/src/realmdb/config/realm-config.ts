import {mediaElementsParams, databaseConfig, PRIMARY_KEY} from "./database-params";
import { getEnvValue } from "../../../config/config";
import { User, Configuration, SyncConfiguration } from "realm"; // Import Realm SDK types
import { logError } from "../../../../utils/logger";

// Define the type for supported data points
export interface SupportedDataPoints {
  [key: string]:
    | "boolean"
    | "string"
    | "float"
    | "double"
    | "int8"
    | "int16"
    | "uint8"
    | "uint16";
}

// Define the Realm schema structure
interface RealmSchema {
  primaryKey: string;
  name: string;
  properties: Record<string, string>;
}

/**
 * Creates a Realm schema for media elements based on the supported endpoints.
 *
 * @param supportedEndpoints - An object representing the data points
 * that are supported, where keys are the property names and values are their data types.
 * @returns The constructed Realm schema object containing the primary key,
 * name of the schema, and properties with their corresponding data types.
 * @throws Throws an error if an unsupported data type is encountered in the
 * provided supportedEndpoints.
 */
function createMediaElementSchema(
  supportedEndpoints: SupportedDataPoints,
): RealmSchema {
  const properties: Record<string, string> = { _id: "string" };

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
          `The initialized data points contain an unsupported data type: ${value}`,
        );
    }
  });

  return {
    primaryKey: PRIMARY_KEY,
    name: mediaElementsParams.VSS.databaseName, // Assuming mediaElementsParams contains 'VSS'
    properties: properties,
  };
}

// Get the schema version from environment variables
const getSchemaVersion = (): number => {
  const schemaVersion = parseInt(
    getEnvValue("VERSION_REALMDB_SCHEMA") ?? "",
    10,
  );
  if (isNaN(schemaVersion)) {
    throw new Error(
      "Version must be specified as an ENV variable and it must be 0 or a positive integer",
    );
  }
  return schemaVersion;
};

/**
 * Configures the Realm database settings.
 *
 * @param user - The user object for authentication.
 * @param  supportedDataPoints - List of supported datapoints for the media element schema.
 * @return The configuration object for the Realm database.
 */
export function realmConfig(
  user: User,
  supportedDataPoints: SupportedDataPoints,
): Configuration {
  const mediaElementSchema = createMediaElementSchema(supportedDataPoints);

  return {
    schema: [mediaElementSchema],
    path: databaseConfig!.storePath,
    sync: {
      user: user,
      flexible: true,
      error: (error: Error) => {
        logError("Realm sync error:", error);
      },
    } as SyncConfiguration, // Cast sync as SyncConfiguration
    schemaVersion: getSchemaVersion(),
    // migration: (oldRealm: any, newRealm: any) => {
    //   // Migration logic here
    // },
  };
}
