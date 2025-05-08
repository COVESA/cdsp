#include "test_helper.h"

#include "data_points_utils.h"
#include "helper.h"
#include "random_utils.h"

/**
 * @brief Generates a JSON data tag based on the provided nodes and data structure type.
 *
 * This function creates a JSON object from a map of nodes, where each node is associated
 * with a variant value that can be a string, int, double, float, or bool. The structure
 * of the JSON object is determined by the specified DataStructure type.
 *
 * @param nodes A map where each key is a node name and the value is a variant containing
 *              a possible data type (string, int, double, float, bool).
 * @param type The type of data structure to generate. It can be Leaf, Flat, or Nested.
 *             - Leaf: Generates a JSON object with a single random value.
 *             - Flat: Generates a flat JSON object with key-value pairs.
 *             - Nested: Generates a nested JSON object based on dot-separated node names.
 *
 * @return A nlohmann::json object representing the generated data tag.
 */
nlohmann::json TestHelper::generateDataTag(
    const std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>&
        nodes,
    DataStructure type) {
    nlohmann::json data;
    if (type == DataStructure::Leaf) {
        // Leaf nodes have a single value
        const auto& value = RandomUtils::generateRandomValue();
        std::visit([&data](const auto& value) { data = value; }, value);
        return data;
    }

    for (const auto& entry : nodes) {
        const std::string& node = entry.first;
        const auto& value = entry.second;
        if (type == DataStructure::Flat) {
            std::visit([&data, &node](const auto& value) { data[node] = value; }, value);
        } else if (type == DataStructure::Nested) {
            std::vector<std::string> segments = Helper::splitString(node, '.');
            nlohmann::json* current = &data;
            for (size_t i = 0; i < segments.size(); ++i) {
                if (i == segments.size() - 1) {
                    std::visit([&current, &segments](
                                   const auto& value) { (*current)[segments.back()] = value; },
                               value);
                } else {
                    if (!current->contains(segments[i])) {
                        (*current)[segments[i]] = nlohmann::json::object();
                    }
                    current = &((*current)[segments[i]]);
                }
            }
        }
    }
    return data;
}

/**
 * Generates a path tag based on the provided data structure type.
 *
 * @param type The type of data structure for which the path tag is to be generated.
 *             It can be either `Leaf` or another type defined in `TestHelper::DataStructure`.
 * @return An optional string containing the generated path tag. If the type is `Leaf`,
 *         a random key is generated and returned. For other types, a random boolean
 *         determines whether a random key is returned or an empty optional.
 */
std::optional<std::string> TestHelper::generatePathTag(TestHelper::DataStructure type) {
    if (type == TestHelper::DataStructure::Leaf) {
        return DataPointsUtils::generateRandomKey();
    }
    return RandomUtils::generateRandomBool()
               ? std::optional<std::string>(DataPointsUtils::generateRandomKey())
               : std::nullopt;
}

/**
 * Generates a metadata map with optional timestamp entries.
 *
 * This function creates a map where each key is a string and each value is a pair of integers
 * representing seconds and nanoseconds. The map may contain the following entries:
 * - "generated": A timestamp indicating when the data was generated, if a random condition is met.
 * - "received": A timestamp indicating when the data was received, if a random condition is met.
 *
 * The presence of each entry is determined by a random boolean value.
 *
 * @return A map containing zero, one, or both of the "generated" and "received" timestamp entries.
 */
std::map<std::string, std::pair<int64_t, int64_t>> TestHelper::generateMetadata() {
    std::map<std::string, std::pair<int64_t, int64_t>> metadata;
    if (RandomUtils::generateRandomBool()) {
        metadata["generated"] = getSecondsAndNanoseconds();
    }
    if (RandomUtils::generateRandomBool()) {
        metadata["received"] = getSecondsAndNanoseconds();
    }
    return metadata;
}

/**
 * Generates a random pair of seconds and nanoseconds.
 *
 * This function generates a random timestamp within a specified range
 * and returns it as a pair of seconds and nanoseconds. The seconds are
 * generated between the start time of 01-01-2025 00:00:00 UTC and the
 * end time of 31-12-2030 23:59:59 UTC. The nanoseconds are generated
 * between 0 and 999,999,999.
 *
 * @return A std::pair containing:
 *         - int64_t: Randomly generated seconds since the epoch.
 *         - int64_t: Randomly generated nanoseconds.
 */
std::pair<int64_t, int64_t> TestHelper::getSecondsAndNanoseconds() {
    int64_t start_time = 1735689600;  // 01-01-2025 00:00:00 UTC
    int64_t end_time = 1924905599;    // 31-12-2030 23:59:59 UTC

    int64_t random_seconds = RandomUtils::generateRandomInt64(start_time, end_time);
    int64_t random_nanoseconds = RandomUtils::generateRandomInt64(0, 999999999);
    return std::make_pair(random_seconds, random_nanoseconds);
}

/**
 * Generates a request ID tag.
 *
 * @return An optional string containing a randomly generated request ID tag
 *         if a random boolean condition is met; otherwise, returns std::nullopt.
 */
std::optional<std::string> TestHelper::generateRequestIdTag() {
    return RandomUtils::generateRandomBool()
               ? std::optional<std::string>(RandomUtils::generateRandomString(8))
               : std::nullopt;
}