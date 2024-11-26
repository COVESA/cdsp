# Knowledge Layer - WebSocket Client

This project contains a WebSocket client built in C++17, which communicates with a WebSocket server. The client can send and receive JSON messages using Boost libraries.

## Prerequisites

Before setting up the project, make sure you have the following installed:

- **CMake** (version 3.25 or higher)
- **Boost** (version 1.86.0 or higher)
- **g++/clang++** with C++17 support
- **Homebrew** (for macOS users)
- A WebSocket server to connect to, see how to start the **information-layer** Websocket server [here](../information-layer/README.md)
  
### Installing Dependencies

1. **Install g++**:
   On windows you may use mingw, e.g. you can install mingw64 with msys64 from here: https://www.msys2.org/
   
   Open the MSYS2 MinGW64 Shell. Make sure you're using the MSYS2 MinGW64 Shell (not the regular MSYS2 shell), as this is required for using the 64-bit GCC toolchain. You can find this in the Start menu as MSYS2 MinGW 64-bit. Run the following command:

    ```bash
    pacman -Syu  # Update MSYS2 packages
    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make  # Install 64-bit GCC and Make  
    ```
    After the installation, verify that gcc and g++ are installed and accessible in the MSYS2 MinGW64 shell by running:
    
    ```bash
    gcc --version
    g++ --version
    ```
    You should see the version information for the installed GCC toolchain.
    

2. **Install Boost**:
   If you're on macOS, you can install Boost using Homebrew:

   ```bash
   brew install boost
   ```

   For Linux, you can install Boost using your package manager, or build from source:
   
   ```bash
   sudo apt-get install libboost-all-dev
   ```

   On Windows you can download a zipfile or take an installer, e.g. from here: https://sourceforge.net/projects/boost/files/boost-binaries/. To use all functionality you have to build the boost library with the gcc compiler.

    Open the MSYS2 MinGW64 Shell from the Start menu (not the standard MSYS2 shell), navigate to your Boost directory and build:

   ```bash
   cd /c/path/to/boost_1_86_0
   ./bootstrap.sh gcc
   ./b2 --with-system --with-filesystem --with-thread
   ```   

   Important Note: When you want to build the projekt with cmake later, cmake often expects specific library names. So if it does not find the libraries, you have to rename them, e.g.: 


   **libboost_system-mgw14-mt-x64-1_86.a** to **libboost_system.a** for the release version and
   **libboost_system-mgw14-mt-d-x64-1_86.a** tp **libboost_system-d.a** for the debug version.
   
   And the same for filesystem, thread, atomic and chrono.

3. **Install CMake**:
   Ensure you have CMake installed:

   ```bash
   brew install cmake  # For macOS
   sudo apt-get install cmake  # For Linux
   ```

   On Windows you can get an installer here: https://cmake.org/download/. 
   
   Make sure you have Windows SDK 10 installed. To check start the Visual Studio Installer (https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/) and look for Visual Studio Build Tools. Here look for the component 'Desktop development with C++' and install it if required.

## Project Setup

**Build the WebSocket Client**:

   The WebSocket client is located in the `cdsp/knowledge_layer/` directory. To build it from this directory:

   ```bash
   # Create a build directory
   mkdir -p build
   cd build

   # Run CMake to generate build files
   # For Mac:
   cmake ..
   # For Windows you have to specify the location of the boost libraries:
   cmake -DBOOST_ROOT=<path/to/boost> -DBOOST_LIBRARYDIR=<path/to/boost>/stage/lib ..

   # Build the project
   make
   ```

   If the build is successful, the WebSocket client executable will be generated in the `build/bin/` directory.

## Running the Reasoner WebSocket Client

The [Reasoner client](./connector/websocket-client/README.md) integrates with both the WebSocket server and the RDFox triple store. The client processes incoming messages and generates RDF triples, which are then stored in RDFox.

### Websocket Server and RDFox Configuration

The project requires certain environment variables to work with the WebSocket server and RDFox server. By default, the WebSocket client connects to the Web Socket server located in the [`information-layer`](../information-layer/README.md), and the RDFox server started as a [Docker container](/docker/README.md#rdfox-restful-api).

- **HOST_WEBSOCKET_SERVER:** Specifies the hostname of the WebSocket server. The default is `127.0.0.1`.
- **PORT_WEBSOCKET_SERVER:** Specifies the port for connecting to the WebSocket server. The default is `8080`.
- **OBJECT_ID:** The object id is required to subscribe and retrieve information for a specific object (e.g. VIN (Vehicle Identification Number) for VSS (Vehicle Signal Specification) data). Use the object id (in this case VIN) configured in the [`information-layer`](../information-layer/README.md).
- **HOST_RDFOX_SERVER:** Hostname of the RDFox server. The default is `127.0.0.1`.
- **PORT_RDFOX_SERVER:** Port for RDFox server. The default is `12110`.
- **AUTH_RDFOX_SERVER_BASE64:** Base64-encoded credentials for RDFox authentication. The default is `cm9vdDphZG1pbg==` (For `root:admin` encoded in base64).
- **RDFOX_DATASTORE:** Data store used in RDFox server to store the generated data. The default is `ds-test`.

You can customize the WebSocket server configuration by adding the following environment variables in the `/docker/.env` file. Below is an example of what the file could look like:

```text
##################################
# WEBSOCKET-SERVER CONFIGURATION #
##################################

HOST_WEBSOCKET_SERVER="your_custom_host"
PORT_WEBSOCKET_SERVER="your_custom_port"
OBJECT_ID="OBJECT_ID_TO_SUBSCRIBE"

##################################
# RDFox CONFIGURATION            #
##################################

HOST_RDFOX_SERVER="your_custom_rdfox_server_host"
PORT_RDFOX_SERVER="your_custom_rdfox_server_port"
AUTH_RDFOX_SERVER_BASE64="your_custom_rdfox_server_authentication"
RDFOX_DATASTORE="your_custom_rdfox_server_data_store"
```

### Start the Reasoner Websocket Client
After successfully building the client, you can run it with the following command (VIN is required, if any docker for the knowledge layer is running):

```bash
OBJECT_ID=<VIN_TO_SUBSCRIBE> ./build/bin/reasoner_client
```

To display a list of available environment variables and their default values, run the application with the `--help` flag:

```bash
./reasoner_client --help
```

### Using RDFox API in the Client

The WebSocket client integrates with the [TripleAssembler](./connector/json-rdf-convertor/rdf-writer/README.md) component, which utilizes the[RDFox API](/docker/README.md#rdfox-restful-api) to process incoming data into RDF triples. The triples are then stored in RDFox for reasoning and querying.

This integration allows the Reasoner client to interpret data, create RDF triples, and store them in a semantic knowledge graph, enabling rich queries and reasoning capabilities.