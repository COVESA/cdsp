# Define the WebSocket client target
add_executable(websocket_client main.cpp websocket_client.cpp)

# Link Boost libraries
target_link_libraries(websocket_client Boost::system Boost::filesystem Boost::thread)

# Link nlohmann/json
target_link_libraries(websocket_client nlohmann_json::nlohmann_json)

# Use defined output directory for the executable
set_target_properties(websocket_client PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)