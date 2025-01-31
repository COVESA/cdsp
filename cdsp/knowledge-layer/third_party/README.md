# Third-Party Libraries

This directory contains third-party libraries that are used in the project. These libraries are included directly in the project to avoid runtime dependencies on external libraries and to ensure consistent builds.

## GeographicLib

GeographicLib is a library for performing geographic calculations. The necessary files from GeographicLib have been included in this directory to provide functionality for calculating Norwegian Transverse Mercator (NTM) coordinates.

### Included Files

#### Header Files
- `include/GeographicLib/Config.h`
- `include/GeographicLib/Constants.hpp`
- `include/GeographicLib/EllipticFunction.hpp`
- `include/GeographicLib/Math.hpp`
- `include/GeographicLib/TransverseMercator.hpp`
- `include/GeographicLib/TransverseMercatorExact.hpp`

#### Source Files
- `src/EllipticFunction.cpp`
- `src/Math.cpp`
- `src/TransverseMercator.cpp`
- `src/TransverseMercatorExact.cpp`

### Directory Structure

```plaintext
third_party/
└── GeographicLib/
    ├── include/
    │   └── GeographicLib/
    │       ├── Config.h
    │       ├── Constants.hpp
    │       ├── EllipticFunction.hpp
    │       ├── Math.hpp
    │       ├── TransverseMercator.hpp
    │       └── TransverseMercatorExact.hpp
    └── src/
        ├── EllipticFunction.cpp
        ├── Math.cpp
        ├── TransverseMercator.cpp
        └── TransverseMercatorExact.cpp
```
### Usage
To use the GeographicLib in your code, include the necessary header files:

```cpp
#include <GeographicLib/TransverseMercator.hpp>
```

Ensure that the `geographiclib` target is linked in your project's `CMakeLists.txt`:

#### Main CMakeLists.txt
```cmake
# Include directories for third-party libraries
include_directories(${CMAKE_SOURCE_DIR}/third_party/GeographicLib/include)

# Add the GeographicLib source files to the project
add_library(geographiclib STATIC
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/TransverseMercator.cpp
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/TransverseMercatorExact.cpp
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/EllipticFunction.cpp
    ${CMAKE_SOURCE_DIR}/third_party/GeographicLib/src/Math.cpp
)
```

And that the geographiclib target is linked in your sub CMakeLists.txt:
```cmake
target_link_libraries(your_target_name
    PRIVATE 
        geographiclib
)
```

# Adding New Libraries
To add a new third-party library, follow these steps:

- Create a new directory for the library under `third_party/`.
- Add the necessary header and source files to the new directory.
- Update the main [`CMakeLists.txt`](/cdsp/knowledge-layer/CMakeLists.txt) to include the new library.
- Link the new library to the appropriate targets in the subdirectories.

## Example: Adding `ExampleLib`

1. **Create Directory**:
    ```bash
    mkdir -p third_party/ExampleLib/include/ExampleLib
    mkdir -p third_party/ExampleLib/src
    ```

2. **Add Files**:
    - Add `ExampleLib.hpp` to `third_party/ExampleLib/include/ExampleLib/`
    - Add `ExampleLib.cpp` to `third_party/ExampleLib/src/`

3. **Update `CMakeLists.txt`**:
    ```cmake
    # Include directories for third-party libraries
    include_directories(${CMAKE_SOURCE_DIR}/third_party/ExampleLib/include)

    # Add the ExampleLib source files to the project
    add_library(examplelib STATIC
        ${CMAKE_SOURCE_DIR}/third_party/ExampleLib/src/ExampleLib.cpp
    )
    ```

4. **Link Library**:
    ```cmake
    target_link_libraries(your_target_name
        PRIVATE 
            examplelib
    )
    ```

By following these steps, you can ensure that all third-party libraries are managed consistently within the project.