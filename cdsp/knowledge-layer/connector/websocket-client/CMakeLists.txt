# Define the websocket_client library
add_library(websocket_client
    src/websocket_client.cpp
    src/real_websocket_connection.cpp
    service/dto_to_bo.cpp
    service/bo_to_dto.cpp
    service/message_utils.cpp
    service/dto_service.cpp
)

# Include directories
target_include_directories(websocket_client 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/service
        ${Boost_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(websocket_client
    PUBLIC 
        nlohmann_json::nlohmann_json
        data_objects
        rdf_writer
    PRIVATE  
        Boost::system
        Boost::filesystem
        Boost::thread
)

# On Windows link winsock, too
if(WIN32)
    target_link_libraries(websocket_client ws2_32)
endif()
