# Add the unit test executable for RDFoxAdapter
add_executable(rdfox_adapter_unit_tests rdfox_adapter_unit_test.cpp)
target_include_directories(rdfox_adapter_unit_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${PROJECT_ROOT_DIR}/symbolic-reasoner/utils/tests/utils

)
target_link_libraries(rdfox_adapter_unit_tests 
    PRIVATE 
        GTest::gtest_main 
        GTest::gmock 
        reasoner
        test_fixtures
)

# Add the integration test executable for RDFoxAdapter
add_executable(rdfox_adapter_integration_tests rdfox_adapter_integration_test.cpp)
target_link_libraries(rdfox_adapter_integration_tests 
    PRIVATE 
        GTest::gtest_main 
        reasoner
        test_fixtures
        utils        
)

# Add unit and integration tests to CTest
add_test(NAME RDFoxAdapterUnitTests COMMAND rdfox_adapter_unit_tests)
add_test(NAME RDFoxAdapterIntegrationTests COMMAND rdfox_adapter_integration_tests)

# Define custom output directory for test binaries
set_target_properties(rdfox_adapter_unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(rdfox_adapter_integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")

# Ensure tests are built with the all target
add_custom_target(symbolic_reasoner_rdfox_tests ALL DEPENDS rdfox_adapter_unit_tests rdfox_adapter_integration_tests)