# Define the websocket_client library
add_library(data_objects    
    bo/data_message.cpp
    bo/get_message.cpp
    bo/message_header.cpp
    bo/node.cpp
    bo/status_message.cpp
    bo/subscribe_message.cpp
    bo/metadata.cpp
    bo/model_config.cpp
    bo/reasoner_settings.cpp
    bo/triple_assembler_helper.cpp
    bo/set_message.cpp
)

# Include directories
target_include_directories(data_objects 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/dto
        ${CMAKE_CURRENT_SOURCE_DIR}/bo
)

# Link dependencies
target_link_libraries(data_objects
    PRIVATE  
        nlohmann_json::nlohmann_json
        utils
)

# Add dependencies
add_dependencies(data_objects utils)
