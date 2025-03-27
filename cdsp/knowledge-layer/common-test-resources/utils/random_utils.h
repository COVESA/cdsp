#ifndef RANDOM_UTILS_H
#define RANDOM_UTILS_H

#include <chrono>
#include <optional>
#include <string>
#include <variant>

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
    static std::chrono::system_clock::time_point generateRandomTimestamp(int start_year = 2000,
                                                                         int end_year = 2030,
                                                                         bool includeNanos = false);
};

#endif  // RANDOM_UTILS_H