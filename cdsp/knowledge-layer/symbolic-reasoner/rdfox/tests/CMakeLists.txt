# GoogleTest integration
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
# Disable installing gtest (we don't need to install it)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add the RDFox adapter source files
add_library(rdfox_adapter STATIC ../src/rdfox-adapter.cpp)
target_include_directories(rdfox_adapter PUBLIC ../src)
target_link_libraries(rdfox_adapter Boost::system Boost::filesystem Boost::thread nlohmann_json)

# Add the unit test executable
add_executable(rdfox_unit_tests test_rdfox_adapter_unit.cpp)
target_include_directories(rdfox_unit_tests PUBLIC ../src)
target_link_libraries(rdfox_unit_tests GTest::gtest GTest::gtest_main GTest::gmock rdfox_adapter)

# Add the integration test executable
add_executable(rdfox_integration_tests test_rdfox_adapter_integration.cpp)
target_include_directories(rdfox_integration_tests PUBLIC ../src)
target_link_libraries(rdfox_integration_tests GTest::gtest GTest::gtest_main rdfox_adapter)

# Add unit and integration tests to CTest
add_test(NAME RDFoxAdapterUnitTests COMMAND rdfox_unit_tests)
add_test(NAME RDFoxAdapterIntegrationTests COMMAND rdfox_integration_tests)

# Define custom output directory for test binaries
set_target_properties(rdfox_unit_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(rdfox_integration_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Ensure tests are built with the all target
add_custom_target(tests ALL
    DEPENDS rdfox_unit_tests rdfox_integration_tests
)