# Define the WebSocket client target
add_executable(websocket_client main.cpp websocket_client.cpp ../utils/message_utils.cpp ../utils/helper.cpp ../utils/model_config.cpp)

# Link libraries
target_link_libraries(websocket_client Boost::system Boost::filesystem Boost::thread)
target_link_libraries(websocket_client nlohmann_json::nlohmann_json)
target_include_directories(websocket_client PRIVATE ${CMAKE_SOURCE_DIR}/connector/utils)


# On Windows link winsock, too
if(WIN32)
    target_link_libraries(websocket_client ws2_32)
endif()

# Use defined output directory for the executable
set_target_properties(websocket_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)