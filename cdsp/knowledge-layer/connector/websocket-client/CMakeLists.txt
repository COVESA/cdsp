# Define the websocket_client library
add_library(websocket_client
    websocket_client.cpp
    real_websocket_connection.cpp
    ../utils/message_utils.cpp
    ../utils/model_config_utils.cpp
)

# Include directories
target_include_directories(websocket_client 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link dependencies
target_link_libraries(websocket_client
    PUBLIC  
        rdf_writer
        Boost::system
        Boost::filesystem
        Boost::thread
        nlohmann_json::nlohmann_json
)

# On Windows link winsock, too
if(WIN32)
    target_link_libraries(websocket_client ws2_32)
endif()
