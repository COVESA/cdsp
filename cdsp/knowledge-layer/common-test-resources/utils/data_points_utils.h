#ifndef DATA_POINTS_UTILS_H
#define DATA_POINTS_UTILS_H

#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class DataPointsUtils {
   public:
    static std::string generateRandomKey(size_t max_segments = 3);
    static std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>
    generateDataPointsWithValues(size_t num_points = 5);
};

#endif  // DATA_POINTS_UTILS_H