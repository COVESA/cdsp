#ifndef UTC_DATE_UTILS_H
#define UTC_DATE_UTILS_H

#include <ctime>
#include <random>
#include <string>

class UtcDateUtils {
   public:
    static std::string generateRandomUtcDate(int start_year = 2000, int end_year = 2030);

   private:
    static std::string formatAsIso8601(const std::tm& time_struct, int fractional_seconds);
};

#endif  // UTC_DATE_UTILS_H