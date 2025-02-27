#ifndef UTC_DATE_UTILS_H
#define UTC_DATE_UTILS_H

#include <chrono>
#include <ctime>
#include <random>
#include <string>

class UtcDateUtils {
   public:
    static std::string getCurrentUtcDate();
    static std::string formatCustomTimestampAsIso8601(
        const std::chrono::system_clock::time_point& timestamp);

   private:
    static std::string formatAsIso8601(const std::tm& time_struct, int fractional_seconds);
};

#endif  // UTC_DATE_UTILS_H