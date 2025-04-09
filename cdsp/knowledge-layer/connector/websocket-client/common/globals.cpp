#include "globals.h"

#include <iostream>

namespace {
std::string PROJECT_ROOT_RUNTIME;
bool isSet = false;
}  // namespace

/**
 * @brief Retrieves the project root directory.
 *
 * This function returns a constant reference to a string representing
 * the project root directory. If the project root is set, it returns
 * the runtime project root; otherwise, it returns the default root.
 *
 * @return const std::string& A constant reference to the project root string.
 */
const std::string& getProjectRoot() {
    static const std::string default_root = std::string(PROJECT_ROOT);
    return isSet ? PROJECT_ROOT_RUNTIME : default_root;
}

/**
 * @brief Sets the project root directory.
 *
 * This function sets the runtime project root directory to the specified
 * path if it has not already been set. If the project root is already
 * set, the function returns false without making any changes.
 *
 * @param root A constant reference to a string representing the new
 * project root directory.
 * @return true if the project root was successfully set;
 *         false if it was already set.
 */
bool setProjectRoot(const std::string& root) {
    if (isSet)
        return false;
    PROJECT_ROOT_RUNTIME = root;
    isSet = true;
    return true;
}