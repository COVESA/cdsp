# Define and link Serd to your example executable
add_executable(serd_triple_writer_example serd_triple_writer_example.cpp)
target_link_libraries(serd_triple_writer_example
    PRIVATE
        ${SERD_LIBRARIES} 
        Boost::system 
        Boost::filesystem 
        Boost::thread 
        nlohmann_json
)

# Use defined output directory for the executable
set_target_properties(serd_triple_writer_example PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests)