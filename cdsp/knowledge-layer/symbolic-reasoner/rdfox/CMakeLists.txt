# Define the rdfox library
add_library(rdfox
    src/rdfox_adapter.cpp
)

# Include directories
target_include_directories(rdfox 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(rdfox 
    PRIVATE 
        Boost::system 
        Boost::thread
)