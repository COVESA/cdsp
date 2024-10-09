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

To display a list of available environment variables and their default values, run the application with the `--help` flag:

```bash
./websocket_client --help
```