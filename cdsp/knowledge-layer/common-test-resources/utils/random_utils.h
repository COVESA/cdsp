#ifndef RANDOM_UTILS_H
#define RANDOM_UTILS_H

#include <chrono>
#include <optional>
#include <string>
#include <variant>

constexpr int DEFAULT_START_YEAR = 2000;
constexpr int DEFAULT_END_YEAR = 2030;

struct TimestampRange {
    int start_year = DEFAULT_START_YEAR;
    int end_year = DEFAULT_END_YEAR;
};

/**
 * @brief Provides utility functions to generate random values.
 */
class RandomUtils {
   public:
    static float generateRandomFloat(float min, float max);
    static int generateRandomInt(int min, int max);
    static int64_t generateRandomInt64(int64_t min, int64_t max);
    static std::string generateRandomString(std::optional<size_t> length = std::nullopt);
    static double generateRandomDouble(double min, double max);
    static std::variant<std::string, int, double, float, bool> generateRandomValue();
    static bool generateRandomBool();
    static std::chrono::system_clock::time_point generateRandomTimestamp(TimestampRange range,
                                                                         bool includeNanos = false);
};

#endif  // RANDOM_UTILS_H