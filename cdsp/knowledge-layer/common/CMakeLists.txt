# Header-only common target with propagated includes
add_library(common 
    globals.cpp
)

target_include_directories(common
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
