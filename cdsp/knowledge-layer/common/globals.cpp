
#include "globals.h"

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::string PROJECT_ROOT_RUNTIME;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
bool IS_PROJECT_ROOT_SET = false;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::string JSON_RPC_VERSION = "2.0";
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::string USE_CASES_PATH = "/symbolic-reasoner/examples/use-case/model/";
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
const std::string &getProjectRoot() {
    static const std::string default_root = std::string(PROJECT_ROOT);
    return IS_PROJECT_ROOT_SET ? PROJECT_ROOT_RUNTIME : default_root;
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
bool setProjectRoot(const std::string &root) {
    if (IS_PROJECT_ROOT_SET) {
        return false;
    }
    PROJECT_ROOT_RUNTIME = root;
    IS_PROJECT_ROOT_SET = true;
    return true;
}

/**
 * @brief Retrieves the JSON-RPC version.
 *
 * This function returns a constant reference to a string representing
 * the JSON-RPC version.
 *
 * @return const std::string& A constant reference to the JSON-RPC version
 * string.
 */
const std::string &getJsonRpcVersion() {
    static const std::string json_rpc_version = JSON_RPC_VERSION;
    return json_rpc_version;
}

/**
 * @brief Sets the path to use cases.
 *
 * This function sets the path to use cases to the specified string.
 * @param path A constant reference to a string representing the new path
 * to use cases.
 */
void setPathToUseCases(const std::string &path) { USE_CASES_PATH = path; }

/**
 * @brief Retrieves the path to use cases.
 *
 * This function returns a constant reference to a string representing
 * the path to use cases.
 *
 * @return const std::string& A constant reference to the path to use cases.
 */
const std::string &getPathToUseCases() {
    static const std::string use_cases_path = USE_CASES_PATH;
    return use_cases_path;
}