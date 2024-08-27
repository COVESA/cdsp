# Configuration Handler for Endpoints

This module is responsible for handling the configuration of supported endpoints in the application. It supports JSON, YAML, or YML formats for defining the endpoints.

The files in this directory `vss_endpoints.yaml` and `vss_endpoints.json` contain the same endpoint definition (only one of them is necessary to build) and based on that, the system will configure the data schema used during the Database configuration.

> [!WARNING]
> - Before starting the application, ensure that the desired YAML, YML or JSON file is correctly placed in the root directory of the module. This file should contain the definitions of the supported endpoints. The name and extension to use can be configured in the `config.js` file.
> - Ensure that the used file is correctly formatted and contains valid endpoint definitions.
> - **The application will not function correctly if the endpoints file is missing or incorrectly placed.**

## File: `config.js`

### Description

The `config.js` file contains the logic to retrieve the full path to the endpoints configuration file (`vss_endpoints.yaml` in this case). This file is crucial for the application to understand which endpoints are supported and how they should be handled.

### Functions

#### `getEndpointsPath`

This function resolves the root directory path of the current module and joins it with the `ENDPOINTS_FILE` constant to form the full path to the endpoints file.

```javascript
const getEndpointsPath = () => {
  const rootPath = path.resolve(__dirname);
  return path.join(rootPath, ENDPOINTS_FILE);
};
