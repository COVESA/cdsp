# Define the rdf-writer library
add_library(rdf_services
    src/reasoning_query_service.cpp
)

# Include directories
target_include_directories(rdf_services
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(rdf_services
    PUBLIC
        utils
        reasoner
        json_writer      
    PRIVATE
        nlohmann_json::nlohmann_json
)

add_dependencies(rdf_services reasoner utils json_writer)