# Configuration Handler for supported Data Points

This module is responsible for handling the configuration of supported data points in the application. It supports JSON, YAML, or YML formats for defining the data points.

The `vss_data_points.yaml` and `vss_data_points.json` files in the `./schema-files` directory contain the same data point definition (only one of them is necessary to build) and based on that, the system will configure the data schema used during the Database configuration.

> [!WARNING]
>
> - Before starting the application, ensure that the desired YAML, YML or JSON file is correctly placed in the `./schema-files` directory. This file should contain the definitions of the supported data points. The name and extension to use can be configured in the `/docker/.env` file, it uses the ENV variable named `DATA_POINTS_SCHEMA_FILE`. If it is not specified, it uses `vss_data_points.yaml` by default, which is one of the files contained in this repository.   .
> - Ensure that the used file is correctly formatted and contains valid data points definitions.
> - **The application will not function correctly if the data points file is missing or incorrectly placed.**

## File: `config.js`

### Description

The `config.js` file contains the logic to retrieve the data defined in the ENV variables and the full path to the data points schema configuration file (`./schema-files/vss_data_points.yaml` in this case). This file is crucial for the application to understand which data points are supported and how they should be handled.

