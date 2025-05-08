cmake_minimum_required(VERSION 3.25)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 
project(knowledge-layer VERSION 1.0 LANGUAGES CXX)


# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the policy related to changes in handling the FindBoost module
cmake_policy(SET CMP0167 NEW)

# Find Boost libraries
find_package(Boost 1.87 REQUIRED COMPONENTS system filesystem thread)

# FetchContent to include dependencies
include(FetchContent)

# ########################
# nlohmann/json dependency
# ########################

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# ############################
# plugixml dependency
# ############################
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.15
)
FetchContent_MakeAvailable(pugixml)

# ########################
# geographiclib dependency
# ########################

# Include directories for third-party libraries
include_directories(${CMAKE_SOURCE_DIR}/third_party/GeographicLib/include)

# Add the GeographicLib source files to the project
add_library(geographiclib STATIC
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/EllipticFunction.cpp
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/Math.cpp
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/TransverseMercator.cpp
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/TransverseMercatorExact.cpp
)

# #####################
# serd-cmake dependency
# #####################

FetchContent_Declare(
    serd
    GIT_REPOSITORY https://github.com/jtsylve/serd-cmake.git
    GIT_TAG e4ace09 # Hash of the last stable commit
)

# Ensure SERD_BUILD_SERDI is OFF to prevent building serdi executable
set(SERD_BUILD_SERDI OFF CACHE BOOL "" FORCE) 
FetchContent_MakeAvailable(serd)

# Exclude serdi target from all builds if it exists
if (TARGET serdi)
    set_target_properties(serdi PROPERTIES EXCLUDE_FROM_ALL TRUE EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif()

# Set SERD_LIBRARIES variable
set(SERD_LIBRARIES serd)

# #####################
# GoogleTest dependency
# #####################

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

# ####################

# Output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define the project directories
set(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}")
set(TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/tests/outputs/")

# Pass the directories to your C++ code
add_compile_definitions(PROJECT_ROOT="${PROJECT_ROOT_DIR}")
add_compile_definitions(TEST_OUTPUT_DIR="${TEST_OUTPUT_DIR}")

# Enable testing
enable_testing()

# Add subdirectories for project modules and tests
add_subdirectory(connector/json-rdf-convertor/json-writer)
add_subdirectory(connector/json-rdf-convertor/json-writer/tests)
add_subdirectory(connector/json-rdf-convertor/rdf-writer)
add_subdirectory(connector/json-rdf-convertor/rdf-writer/tests)
add_subdirectory(connector/json-rdf-convertor/rdf-writer/tests/serd_library_tests)
add_subdirectory(connector/json-rdf-convertor/services)
add_subdirectory(connector/data-objects)
add_subdirectory(connector/utils)
add_subdirectory(connector/websocket-client)
add_subdirectory(connector/websocket-client/src/tests)
add_subdirectory(connector/websocket-client/services/tests)
add_subdirectory(symbolic-reasoner)
add_subdirectory(symbolic-reasoner/rdfox/tests)
add_subdirectory(symbolic-reasoner/rdfox/tests/rdfox-service-test)
add_subdirectory(symbolic-reasoner/services/tests)
add_subdirectory(common-test-resources)

# Define the main executable target
add_executable(reasoner_client connector/websocket-client/src/main.cpp)

# Link the websocket_client library to main
target_link_libraries(reasoner_client
    PRIVATE 
        websocket_client
)