# Configuration Handler for supported Data Points

This module is responsible for handling the configuration of supported data points in the application. It supports JSON, YAML, or YML formats for defining the data points.

The `vss_data_points.yaml` file in the `./schema-files` directory contains the data point definitions, and based on that the system configures the data schema used during the database configuration. The file is generated from a GraphQL schema by the [S2DM generator](../../../s2dm/README.md) and is not tracked in git; Docker Compose generates it at start-up.

> [!WARNING]
>
> - Before starting the application natively, generate `vss_data_points.yaml` into the `./schema-files` directory (see the [Information Layer README](../../README.md)); the build copies it to the corresponding folder below `../../dist`. The file name is fixed to `vss_data_points.yaml` in `config.ts`.
> - Ensure that the used file is correctly formatted and contains valid data points definitions.
> - **The application will not work correctly if the data points file is missing or incorrectly placed.**

## File: `config.ts`

### Description

The `config.ts` file contains the logic to retrieve the data defined in the ENV variables and the full path to the data points schema configuration file (`./schema-files/vss_data_points.yaml` in this case). This file is crucial for the application to understand which data points are supported and how they should be handled.
