# Define the rdfox library
add_library(rdfox
    src/rdfox_adapter.cpp
    ../utils/request_builder.cpp
)

# Include directories
target_include_directories(rdfox 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
        ${PROJECT_ROOT_DIR}/symbolic-reasoner/utils
)

# Link dependencies
target_link_libraries(rdfox 
    PRIVATE 
        Boost::system 
        Boost::thread
)