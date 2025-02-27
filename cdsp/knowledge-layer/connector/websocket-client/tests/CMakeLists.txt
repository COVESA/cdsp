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
        ${PROJECT_ROOT_DIR}/connector/websocket-client/tests/utils
)

# Add the test for DTO service
add_executable(dto_service_unit_test 
    dto_service_unit_test.cpp
    ${PROJECT_ROOT_DIR}/connector/websocket-client/tests/utils/test_helper.cpp
)
target_link_libraries(dto_service_unit_test 
    PRIVATE 
        GTest::gtest_main 
        websocket_client
        test_fixtures
)
target_include_directories(dto_service_unit_test
    PRIVATE
        ${PROJECT_ROOT_DIR}/connector/websocket-client/tests/utils
)

# Add the test for DTO to BO conversion
add_executable(dto_to_bo_integration_test 
    dto_to_bo_integration_test.cpp
    ${PROJECT_ROOT_DIR}/connector/websocket-client/tests/utils/test_helper.cpp    
)
target_link_libraries(dto_to_bo_integration_test 
    PRIVATE 
        GTest::gtest_main 
        websocket_client
        test_fixtures
)
target_include_directories(dto_to_bo_integration_test
    PRIVATE
        ${PROJECT_ROOT_DIR}/connector/websocket-client/tests/utils
        ${PROJECT_ROOT_DIR}/connector/utils
)

# Add the test for BO to DTO conversion
add_executable(bo_to_dto_unit_test 
bo_to_dto_unit_test.cpp
    ${PROJECT_ROOT_DIR}/connector/websocket-client/tests/utils/test_helper.cpp    
)
target_link_libraries(bo_to_dto_unit_test 
    PRIVATE 
        GTest::gtest_main 
        websocket_client
        test_fixtures
)
target_include_directories(bo_to_dto_unit_test
    PRIVATE
        ${PROJECT_ROOT_DIR}/connector/websocket-client/tests/utils
)

# Add unit and integration tests to CTest
add_test(NAME WebsocketClientIntegrationTests COMMAND websocket_client_integration_tests)
add_test(NAME DtoServiceUnitTest COMMAND dto_service_unit_test)  
add_test(NAME DtoToBoIntegrationTests COMMAND dto_to_bo_integration_test)
add_test(NAME BoToDtoUnitTests COMMAND bo_to_dto_unit_test)

# Define custom output directory for test binaries
set_target_properties(websocket_client_integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(dto_service_unit_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests") 
set_target_properties(dto_to_bo_integration_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(bo_to_dto_unit_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")  # Added this line

# Ensure tests are built with the all target
add_custom_target(websocket_client_tests ALL DEPENDS websocket_client_integration_tests dto_service_unit_test dto_to_bo_integration_test)  