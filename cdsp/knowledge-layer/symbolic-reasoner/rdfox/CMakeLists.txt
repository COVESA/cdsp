add_library(rdfox
    src/rdfox_adapter.cpp
    ../utils/request_builder.cpp
)

target_include_directories(rdfox 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/symbolic-reasoner/utils
)

target_link_libraries(rdfox
    PUBLIC
        utils
    PRIVATE 
        Boost::system 
        Boost::thread
)