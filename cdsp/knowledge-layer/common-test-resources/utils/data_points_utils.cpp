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
 * @brief Generates a map of data points with random values.
 *
 * This function creates a specified number of data points, each associated with a random key
 * and a random value. The values can be of various types including std::string, int, double,
 * float, and bool.
 *
 * @param num_points The number of data points to generate.
 * @return A map where each key is a randomly generated string and each value is a randomly
 * generated variant of types std::string, int, double, float, or bool.
 */
std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>
DataPointsUtils::generateDataPointsWithValues(size_t num_points) {
    std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>
        data_points;
    for (size_t i = 0; i < num_points; ++i) {
        data_points[generateRandomKey()] = RandomUtils::generateRandomValue();
    }
    return data_points;
}