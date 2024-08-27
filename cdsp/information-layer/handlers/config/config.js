const path = require("path");

/**
 * This file contains the description of the supported endpoints.
 * It supports JSON, YAML or YML format.
 */
const ENDPOINTS_FILE = "vss_endpoints.yaml";

/**
 * Retrieves the full path to the endpoints file.
 *
 * This function resolves the root directory path of the current module
 * and joins it with the ENDPOINTS_FILE constant to form the full path.
 *
 * @returns {string} The full path to the endpoints file.
 */
const getEndpointsPath = () => {
  const rootPath = path.resolve(__dirname);
  return path.join(rootPath, ENDPOINTS_FILE);
};

module.exports = { getEndpointsPath };
