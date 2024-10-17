cmake_minimum_required(VERSION 3.10)

project(knowledge-layer)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# Find Boost libraries
find_package(Boost 1.86.0 REQUIRED COMPONENTS system filesystem thread)

if(WIN32)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# FetchContent to include nlohmann/json
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define the project root directory
set(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}")

# Pass the root directory to your C++ code
add_compile_definitions(PROJECT_ROOT="${PROJECT_ROOT_DIR}")

enable_testing()

# Add subdirectory for the websocket-client
add_subdirectory(connector/websocket-client)
add_subdirectory(symbolic-reasoner/rdfox/rdfox-service-test)
add_subdirectory(symbolic-reasoner/rdfox/tests)