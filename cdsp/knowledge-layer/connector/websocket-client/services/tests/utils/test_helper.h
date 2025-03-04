#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

class TestHelper {
   public:
    enum class DataStructure { Flat, Nested, Leaf };
    static nlohmann::json generateDataTag(
        const std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>&
            nodes,
        DataStructure type);

    static std::optional<std::string> generatePathTag(DataStructure type);
    static std::map<std::string, std::pair<int64_t, int64_t>> generateMetadata();
    static std::pair<int64_t, int64_t> getSecondsAndNanoseconds();
    static std::optional<std::string> generateRequestIdTag();
};  // class TestHelper

#endif  // TEST_HELPER_H