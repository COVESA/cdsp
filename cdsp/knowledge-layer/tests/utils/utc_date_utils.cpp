#include "utc_date_utils.h"

#include <iomanip>
#include <sstream>

/**
 * @brief Generates a random UTC date as an ISO 8601 string.
 *
 * @param start_year The earliest possible year for the random date.
 * @param end_year The latest possible year for the random date.
 * @return A random UTC date in ISO 8601 format.
 */
std::string UtcDateUtils::generateRandomUtcDate(int start_year, int end_year) {
    // Random number generator
    static std::random_device rd;
    static std::mt19937 gen(rd());

    // Generate random year, month, day, hour, minute, second
    std::uniform_int_distribution<> year_dis(start_year, end_year);
    std::uniform_int_distribution<> month_dis(1, 12);
    std::uniform_int_distribution<> day_dis(1, 28);  // To simplify, avoid invalid dates
    std::uniform_int_distribution<> hour_dis(0, 23);
    std::uniform_int_distribution<> minute_dis(0, 59);
    std::uniform_int_distribution<> second_dis(0, 59);

    // Populate tm structure
    std::tm time_struct{};
    time_struct.tm_year = year_dis(gen) - 1900;  // tm_year is years since 1900
    time_struct.tm_mon = month_dis(gen) - 1;     // tm_mon is 0-based
    time_struct.tm_mday = day_dis(gen);
    time_struct.tm_hour = hour_dis(gen);
    time_struct.tm_min = minute_dis(gen);
    time_struct.tm_sec = second_dis(gen);

    return formatAsIso8601(time_struct);
}

/**
 * @brief Helper function to format a tm structure as an ISO 8601 string.
 *
 * @param time_struct The tm structure to format.
 * @return The formatted date-time string in ISO 8601 format.
 */
std::string UtcDateUtils::formatAsIso8601(const std::tm& time_struct) {
    std::ostringstream oss;
    oss << std::put_time(&time_struct, "%Y-%m-%dT%H:%M:%S") << "Z";
    return oss.str();
}