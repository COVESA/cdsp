const path = require("path");
const dotenv = require("dotenv");
dotenv.config();
/**
 * This file contains the description of the supported data points.
 * It supports JSON, YAML or YML format.
 */
const ENDPOINTS_FILE = "vss_data_points.yaml";

/**
 * Retrieves the value of an environment variable.
 *
 * @param {string} envVar - The environment variable to retrieve.
 * @returns {string|null} - The value of the environment variable.
 */
const getEnvValue = (envVar) => {
  if (!process.env[envVar]) {
    console.info(`${envVar} environment variable is not set in .env file.`);
    return null;
  }
  return process.env[envVar];
};

/**
 * Retrieves the full path to the data points file.
 *
 * This function resolves the schema-files directory path of the current module
 * and joins it with the ENDPOINTS_FILE constant to form the full path.
 *
 * @returns {string} The full path to the data points file.
 */
const getDataPointsPath = () => {
  const rootPath = path.resolve(`${__dirname}/schema-files`);
  return path.join(rootPath, ENDPOINTS_FILE);
};

module.exports = { getDataPointsPath, getEnvValue };
