# Define the websocket_client_runtime library
add_library(websocket_client_runtime
    request_registry.cpp
)

# Include directories
target_include_directories(websocket_client_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(websocket_client_runtime
    PUBLIC 
        common
        data_objects
        utils
)