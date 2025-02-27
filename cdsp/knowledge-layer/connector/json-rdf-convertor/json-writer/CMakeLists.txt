# Define the rdf-writer library
add_library(json_writer
    src/json_writer.cpp
)

# Include directories
target_include_directories(json_writer 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${serd_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(json_writer
    PUBLIC 
        utils
    PRIVATE 
        nlohmann_json::nlohmann_json
        pugixml::pugixml
)