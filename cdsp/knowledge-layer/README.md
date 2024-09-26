# Knowledge Layer - WebSocket Client

This project contains a WebSocket client built in C++17, which communicates with a WebSocket server. The client can send and receive JSON messages using Boost libraries.

## Prerequisites

Before setting up the project, make sure you have the following installed:

- **CMake** (version 3.10 or higher)
- **Boost** (version 1.86.0 or higher)
- **g++/clang++** with C++17 support
- **Homebrew** (for macOS users)
- A WebSocket server to connect to, see how to start the **information-layer** Websocket server [here](../information-layer/README.md)
  
### Installing Dependencies

1. **Installing Boost (macOS/Linux)**:
   If you're on macOS, you can install Boost using Homebrew:

   ```bash
   brew install boost
   ```

   For Linux, you can install Boost using your package manager, or build from source:
   
   ```bash
   sudo apt-get install libboost-all-dev
   ```

2. **Install CMake**:
   Ensure you have CMake installed:

   ```bash
   brew install cmake  # For macOS
   sudo apt-get install cmake  # For Linux
   ```

## Project Setup

**Build the WebSocket Client**:

   The WebSocket client is located in the `connector/websocket-client/` directory. To build it from this directory:

   ```bash
   # Create a build directory
   mkdir -p build
   cd build

   # Run CMake to generate build files
   cmake ..

   # Build the project
   make
   ```

   If the build is successful, the WebSocket client executable will be generated in the `build/bin/` directory.

## Running the WebSocket Client

### Environment Variables

The project requires certain environment variables to work with the WebSocket server’s host and port. By default, the WebSocket client connects to the `information-layer`. See how to configure these variables [here](../information-layer/README.md).


- **HOST_WEBSOCKET_SERVER:** Specifies the hostname of the WebSocket server. The default is `localhost`.
- **PORT_WEBSOCKET_SERVER:** Specifies the port for connecting to the WebSocket server. The default is `8080`.
- **VIN:** The Vehicle Identification Number (VIN) is required to subscribe and retrieve information for a specific vehicle. Use the VIN configured in the [`information-layer`](../information-layer/README.md).
- **REQUIRED_VSS_DATA_POINTS_FILE:** The name of the TXT file containing all the required data points to start the application. See more details [here](symbolic-reasoner/examples/usecase-model/inputs/README.md). The default value is `vss_data_required.txt`.

You can customize the WebSocket server configuration by adding the following environment variables in the `/docker/.env` file. Below is an example of what the file could look like:

```text
##################################
# WEBSOCKET-SERVER CONFIGURATION #
##################################

HOST_WEBSOCKET_SERVER="your_custom_host"
PORT_WEBSOCKET_SERVER="your_custom_port"
VIN="VIN_TO_SUBSCRIBE"
REQUIRED_VSS_DATA_POINTS_FILE=vss_data_required.txt
```

### Start the Websocket Client
After successfully building the client, you can run it with the following command:

```bash
./build/bin/websocket_client
```
