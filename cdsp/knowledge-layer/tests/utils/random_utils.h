#ifndef RANDOM_UTILS_H
#define RANDOM_UTILS_H

#include <string>

/**
 * @brief Provides utility functions to generate random values.
 */
class RandomUtils {
   public:
    static float generateRandomFloat(float min, float max);
    static int generateRandomInt(int min, int max);
    static std::string generateRandomString(size_t length);
    static double generateRandomDouble(double min, double max);
};

#endif  // RANDOM_UTILS_H