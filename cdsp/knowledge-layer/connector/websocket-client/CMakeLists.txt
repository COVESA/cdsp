# Define the websocket_client library
add_library(websocket_client
    common/globals.cpp
    src/websocket_client.cpp
    src/real_websocket_connection.cpp
    services/bo_service.cpp
    services/bo_to_dto.cpp
    services/message_service.cpp
    services/system_configuration_service.cpp
    services/dto_service.cpp
    services/converters/data_message_converter.cpp
    services/converters/status_message_converter.cpp
    services/converters/model_config_converter.cpp
    services/converters/utils/converter_helper.cpp
)

# Include directories
target_include_directories(websocket_client 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/services
        ${CMAKE_CURRENT_SOURCE_DIR}/services/converters
        ${CMAKE_CURRENT_SOURCE_DIR}/services/converters/utils
        ${Boost_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(websocket_client
    PUBLIC 
        nlohmann_json::nlohmann_json
        data_objects
        rdf_writer
        rdf_services
        reasoner
    PRIVATE  
        Boost::system
        Boost::filesystem
        Boost::thread
)

# On Windows link winsock, too
if(WIN32)
    target_link_libraries(websocket_client ws2_32)
endif()
