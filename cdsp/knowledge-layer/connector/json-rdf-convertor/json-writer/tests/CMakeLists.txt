# Add the unit test executable for JsonWriter
add_executable(json_writer_unit_tests json_writer_unit_test.cpp)
target_link_libraries(json_writer_unit_tests 
    PRIVATE 
        GTest::gtest_main 
        GTest::gmock 
        json_writer
        test_fixtures
        nlohmann_json::nlohmann_json
)
target_include_directories(json_writer_unit_tests
    PRIVATE
        ${PROJECT_ROOT_DIR}/connector/utils/tests/utils
)

# Add the integration test executable for JsonWriter
add_executable(json_writer_integration_tests json_writer_integration_test.cpp)
target_link_libraries(json_writer_integration_tests 
    PRIVATE 
        GTest::gtest_main 
        json_writer
        test_fixtures
        nlohmann_json::nlohmann_json
)

# Add unit and integration tests to CTest
add_test(NAME JsonWriterUnitTests COMMAND json_writer_unit_tests)
add_test(NAME JsonWriterIntegrationTests COMMAND json_writer_integration_tests)

# Define custom output directory for test binaries
set_target_properties(json_writer_unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(json_writer_integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")

# Ensure tests are built with the all target
add_custom_target(json_writer_tests ALL DEPENDS json_writer_unit_tests json_writer_integration_tests)