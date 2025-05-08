# Add the integration test executable for WebsocketClient
add_executable(websocket_client_integration_tests 
    websocket_client_base_integration_test.cpp
    websocket_client_integration_test.cpp
    websocket_client_coordinates_integration_test.cpp
)

target_link_libraries(websocket_client_integration_tests 
    PRIVATE 
        GTest::gtest_main 
        GTest::gmock 
        websocket_client
        test_fixtures
)
target_include_directories(websocket_client_integration_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Add unit and integration tests to CTest
add_test(NAME WebsocketClientIntegrationTests COMMAND websocket_client_integration_tests)

# Define custom output directory for test binaries
set_target_properties(websocket_client_integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")

# Ensure tests are built with the all target
add_custom_target(websocket_client_tests ALL DEPENDS websocket_client_integration_tests)