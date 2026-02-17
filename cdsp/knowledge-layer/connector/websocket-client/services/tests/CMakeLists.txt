# Add the test for ModelConfigDTO service
add_executable(model_config_dto_service_unit_test 
    model_config_dto_service_unit_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/test_helper.cpp
)
target_link_libraries(model_config_dto_service_unit_test 
    PRIVATE 
        GTest::gtest_main 
        websocket_client
        test_fixtures
)
target_include_directories(model_config_dto_service_unit_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Add the test for DTO to Model Config BO conversion
add_executable(dto_to_model_config_integration_test 
dto_to_model_config_integration_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/test_helper.cpp    
)
target_link_libraries(dto_to_model_config_integration_test 
    PRIVATE 
        GTest::gtest_main
        GTest::gmock
        websocket_client
        test_fixtures
)
target_include_directories(dto_to_model_config_integration_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${PROJECT_ROOT_DIR}/connector/utils
        ${PROJECT_ROOT_DIR}/connector/utils/tests/utils
)

# Add the test for BO Service
add_executable(bo_service_integration_test 
    bo_service_integration_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/test_helper.cpp    
)
target_link_libraries(bo_service_integration_test 
    PRIVATE 
        GTest::gtest_main 
        websocket_client
        test_fixtures
)
target_include_directories(bo_service_integration_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Add unit and integration tests to CTest
add_test(NAME ModelConfigDtoServiceUnitTest COMMAND model_config_dto_service_unit_test)  
add_test(NAME DtoToModelConfigIntegrationTest COMMAND dto_to_model_config_integration_test)
add_test(NAME BoServiceIntegrationTest COMMAND bo_service_integration_test)

# Define custom output directory for test binaries
set_target_properties(model_config_dto_service_unit_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests") 
set_target_properties(dto_to_model_config_integration_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")
set_target_properties(bo_service_integration_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/tests")

# Ensure tests are built with the all target
add_custom_target(websocket_client_services_tests ALL DEPENDS  
    dto_to_model_config_integration_test
    model_config_dto_service_unit_test
    bo_service_integration_test )