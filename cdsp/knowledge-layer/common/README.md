# Common Utilities

The `common/` directory provides cross-cutting utilities used throughout the
Knowledge Layer. These helpers cover console logging and global runtime state
so that other modules can rely on a consistent API.

## Components

- `globals.*` – Shared runtime state: project root path, use-case path, and JSON-RPC
  version string. These functions expose getters/setters so services can configure global
  paths during startup.

Both utilities are linked into most submodules via CMake to provide consistent
logging and configuration behavior.

## Usage Example

```cpp
#include "globals.h"
#include <iostream>

int main() {
	setPathToUseCases("/path/to/use-case");
  std::cout << "Use-case path: " << getProjectRoot() << getPathToUseCases() << std::endl;
}
```

## Tests

Modules that depend on these utilities include their usage in integration
tests; there are no dedicated unit tests inside `common/` itself.
