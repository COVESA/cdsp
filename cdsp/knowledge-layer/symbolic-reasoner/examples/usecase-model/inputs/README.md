# Configuration Websocket-Client supported Data Points

This module is responsible for handling the configuration of supported data points in the application. It supports a TXT format for defining the data points.

The `vss_data_required.txt` in this directory contains the data point definitions that websocket-client will use during the initial connection.

> [!WARNING]
>
> - Before starting the application, ensure that the desired TXT file is correctly placed in this directory. This file should contain the definitions of the supported data points. The name and extension to use can be configured in the `/docker/.env` file, it uses the ENV variable named `REQUIRED_VSS_DATA_POINTS_FILE`. If it is not specified, it uses `vss_data_required.txt` by default, which is one of the files contained in this repository.   .
> - Ensure that the used file is correctly formatted and contains valid data points definitions.
> - **The application will not function correctly if the data points file is missing or incorrectly placed.**