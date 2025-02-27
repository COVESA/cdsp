#include "data_points_utils.h"

#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include "random_utils.h"

/**
 * @brief Generates a random key composed of a specified number of segments.
 *
 * This function generates a random key consisting of a random number of segments,
 * each segment being a random string of length between 3 and 7 characters. The segments
 * are concatenated with a period ('.') separator.
 *
 * @param max_segments The maximum number of segments the key can have. Defaults to 3.
 * @return A string representing the generated random key.
 */
std::string DataPointsUtils::generateRandomKey(size_t max_segments) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> segment_count_dist(1, max_segments);
    size_t segment_count = segment_count_dist(rng);

    std::vector<std::string> segments;
    for (size_t i = 0; i < segment_count; ++i) {
        segments.push_back(
            RandomUtils::generateRandomString(3 + rand() % 5));  // Random length between 3-7
    }
    return std::accumulate(segments.begin() + 1, segments.end(), segments[0],
                           [](const std::string& a, const std::string& b) { return a + "." + b; });
}

/**
 * @brief Generates a random value of a random type.
 *
 * This function generates a random value which can be one of the following types:
 * - std::string: A random string of length 8.
 * - int: A random integer between -1000 and 999.
 * - double: A random double between -1000.0 and 1000.0.
 * - bool: A random boolean value (true or false).
 *
 * @return std::variant<std::string, int, double, bool> A variant containing the random value.
 */
std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>
DataPointsUtils::generateDataPoints(size_t num_points) {
    std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>
        data_points;
    for (size_t i = 0; i < num_points; ++i) {
        data_points[generateRandomKey()] = RandomUtils::generateRandomValue();
    }
    return data_points;
}