# Add the unit test executable for RDFoxAdapter
add_executable(rdfox_adapter_unit_tests rdfox_adapter_unit_test.cpp)
target_include_directories(rdfox_adapter_unit_tests
    PRIVATE
        ${PROJECT_ROOT_DIR}/symbolic-reasoner/test/utils
)
target_link_libraries(rdfox_adapter_unit_tests 
    PRIVATE 
        GTest::gtest_main 
        GTest::gmock 
        rdfox
        test_fixtures
)

# Add the integration test executable for RDFoxAdapter
add_executable(rdfox_adapter_integration_tests rdfox_adapter_integration_test.cpp)
target_link_libraries(rdfox_adapter_integration_tests 
    PRIVATE 
        GTest::gtest_main 
        rdfox
        test_fixtures
)
target_include_directories(rdfox_adapter_integration_tests
    PRIVATE
        ${PROJECT_ROOT_DIR}/connector/utils   
)

# Add unit and integration tests to CTest
add_test(NAME RDFoxAdapterUnitTests COMMAND rdfox_adapter_unit_tests)
add_test(NAME RDFoxAdapterIntegrationTests COMMAND rdfox_adapter_integration_tests)

# Define custom output directory for test binaries
set_target_properties(rdfox_adapter_unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(rdfox_adapter_integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")

# Ensure tests are built with the all target
add_custom_target(tests ALL DEPENDS rdfox_adapter_unit_tests rdfox_adapter_integration_tests)