# Define the rdf-writer library
add_library(json_writer
    src/json_writer.cpp
    ${PROJECT_ROOT_DIR}/connector/utils/helper.cpp
    ${PROJECT_ROOT_DIR}/connector/utils/file_handler_impl.cpp    
    ${PROJECT_ROOT_DIR}/connector/utils/coordinate_transform.cpp
)

# Include directories
target_include_directories(json_writer 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${serd_SOURCE_DIR}/include
        ${PROJECT_ROOT_DIR}/connector/utils
)

# Link libraries
target_link_libraries(json_writer
    PRIVATE 
        nlohmann_json::nlohmann_json
        pugixml::pugixml
)