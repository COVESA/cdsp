# Define utils library
add_library(utils
    helper.cpp
    file_handler_impl.cpp
    coordinate_transform.cpp
)

# Link dependencies
target_include_directories(utils
    PUBLIC 
        ${PROJECT_ROOT_DIR}/connector/utils
        ${PROJECT_ROOT_DIR}/third_party/GeographicLib/include
)

# Include directories
target_link_libraries(utils
    PUBLIC 
        nlohmann_json::nlohmann_json
        geographiclib
)

