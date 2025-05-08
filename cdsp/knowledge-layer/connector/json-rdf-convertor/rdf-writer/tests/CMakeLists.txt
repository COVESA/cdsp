# Add the integration test executable for TripleWriter
add_executable(triple_writer_integration_tests triple_writer_integration_test.cpp)
target_link_libraries(triple_writer_integration_tests 
    PRIVATE 
        GTest::gtest_main 
        rdf_writer
        test_fixtures
)

# Add the unit test executable for TripleAssembler
add_executable(triple_assembler_unit_tests triple_assembler_unit_test.cpp)
target_include_directories(triple_assembler_unit_tests
    PRIVATE
        ${PROJECT_ROOT_DIR}/connector/utils/tests/utils
        ${PROJECT_ROOT_DIR}/connector/data-objects/bo/tests/utils
        ${PROJECT_ROOT_DIR}/symbolic-reasoner/interfaces/tests/utils
        ${PROJECT_ROOT_DIR}/symbolic-reasoner/services/tests/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
)
target_link_libraries(triple_assembler_unit_tests 
    PRIVATE 
        GTest::gtest_main 
        GTest::gmock 
        rdf_writer
        test_fixtures        
)


# Add unit and integration tests to CTest
add_test(NAME TripleWriterIntegrationTests COMMAND triple_writer_integration_tests)
add_test(NAME TripleAssemblerUnitTests COMMAND triple_assembler_unit_tests)

# Define custom output directory for test binaries
set_target_properties(triple_writer_integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(triple_assembler_unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")

# Ensure tests are built with the all target
add_custom_target(rdf_writer_tests ALL DEPENDS triple_writer_integration_tests triple_assembler_unit_tests)