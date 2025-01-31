# Define the RDFox test target
add_executable(
    rdfox_test rdfox_test_main.cpp
)
target_link_libraries(rdfox_test Boost::system Boost::filesystem Boost::thread)

# Use defined output directory for the executable
set_target_properties(rdfox_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests)