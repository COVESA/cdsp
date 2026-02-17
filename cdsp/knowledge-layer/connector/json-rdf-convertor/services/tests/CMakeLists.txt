# Add unit test executable for ReasoningQueryService
add_executable(reasoning_query_service_unit_test reasoning_query_service_unit_test.cpp)

target_include_directories(reasoning_query_service_unit_test
    PRIVATE
        ${PROJECT_ROOT_DIR}/symbolic-reasoner/services/tests/utils
        ${PROJECT_ROOT_DIR}/symbolic-reasoner/interfaces/tests/utils
)

target_link_libraries(reasoning_query_service_unit_test
    PRIVATE
        GTest::gtest_main
        GTest::gmock
        rdf_services
        test_fixtures
)

# Add reasoning query service integration test executable
add_executable(reasoning_query_service_integration_test 
    reasoning_query_service_integration_test.cpp
)

target_link_libraries(reasoning_query_service_integration_test
    PRIVATE
        GTest::gtest_main
        test_fixtures
        websocket_client
)

# Add integration test to CTest
add_test(NAME ReasoningQueryServiceUnitTests COMMAND reasoning_query_service_unit_test)
add_test(NAME ReasoningQueryServiceIntegrationTests COMMAND reasoning_query_service_integration_test)

# Define custom output directory for test binaries
set_target_properties(reasoning_query_service_unit_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(reasoning_query_service_integration_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")

# Ensure tests are built with the all target
add_custom_target(json_rdf_convertor_reasoning_query_tests ALL DEPENDS reasoning_query_service_unit_test reasoning_query_service_integration_test)