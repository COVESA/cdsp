#ifndef VIN_UTILS_H
#define VIN_UTILS_H

#include <functional>
#include <string>
#include <vector>

/**
 * @brief Provides utility functions to generate VIN strings.
 */
class VinUtils {
   public:
    static std::string getRandomVinString();

   private:
    static const std::vector<std::string>& getVinList();
};

#endif  // VIN_UTILS_H