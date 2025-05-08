import path from "path";
import dotenv from "dotenv";
import { logMessage } from "../../utils/logger";

// Load environment variables from the .env file
dotenv.config();

/**
 * This file contains the description of the supported data points.
 * It supports JSON, YAML or YML format.
 */
const ENDPOINTS_FILE: string = "vss_data_points.yaml";

// Define the type of the handler, which will be a lowercase string
export const getHandlerType = (): string => {
  const handlerType = process.env.HANDLER_TYPE;

  if (!handlerType) {
    throw new Error("HANDLER_TYPE must be specified as an ENV variable");
  }

  return handlerType.toLowerCase(); // Ensure it's returned as lowercase
};

/**
 * Retrieves the value of an environment variable.
 *
 * @param envVar - The environment variable to retrieve.
 * @returns The value of the environment variable or null if not set.
 */
export const getEnvValue = (envVar: string): string | null => {
  if (!process.env[envVar]) {
    logMessage(`${envVar} environment variable is not set in .env file.`);
    return null;
  }
  return process.env[envVar]!;
};

/**
 * Retrieves the full path to the data points file.
 *
 * This function resolves the schema-files directory path of the current module
 * and joins it with the ENDPOINTS_FILE constant to form the full path.
 *
 * @returns The full path to the data points file.
 */
export const getDataPointsPath = (): string => {
  const rootPath = path.resolve(`${__dirname}/schema-files`);
  return path.join(rootPath, ENDPOINTS_FILE);
};
