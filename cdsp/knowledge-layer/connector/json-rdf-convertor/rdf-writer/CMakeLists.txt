# Define the rdf-writer library
add_library(rdf_writer
    src/triple_assembler.cpp
    src/triple_writer.cpp
)

# Include directories
target_include_directories(rdf_writer
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE
       ${serd_SOURCE_DIR}/include
       ${PROJECT_ROOT_DIR}/third_party/GeographicLib/include
)

# Link libraries
target_link_libraries(rdf_writer
    PUBLIC
        utils
        data_objects
        reasoner        
        ${SERD_LIBRARIES} 
    PRIVATE
        nlohmann_json::nlohmann_json
        geographiclib
)

add_dependencies(rdf_writer reasoner utils data_objects)