# Define the test_fixtures library
add_library(test_fixtures
    common_fixtures/server_data_fixture.cpp
    utils/utc_date_utils.cpp
    utils/vin_utils.cpp
    utils/random_utils.cpp
    utils/observation_id_utils.cpp
    utils/data_points_utils.cpp
)

# Include directories
target_include_directories(test_fixtures 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/common_fixtures
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Link dependencies
target_link_libraries(test_fixtures
    PRIVATE 
        Boost::system
        Boost::filesystem
        Boost::thread    
        nlohmann_json::nlohmann_json
        geographiclib
        utils
)

