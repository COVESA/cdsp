#include "utc_date_utils.h"

#include <iomanip>
#include <random>
#include <sstream>

/**
 * @brief Gets the current UTC date as an ISO 8601 string with fractional seconds.
 *
 * @return The current UTC date in ISO 8601 format with fractional seconds.
 */
std::string UtcDateUtils::getCurrentUtcDate() {
    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::gmtime(&now_time_t);

    // Get the fractional seconds
    auto now_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    int fractional_seconds = now_ns % 1000000000;

    return formatAsIso8601(now_tm, fractional_seconds);
}

/**
 * @brief Formats a custom timestamp as an ISO 8601 string with fractional seconds.
 *
 * @param timestamp The timestamp to format.
 * @return The formatted date-time string in ISO 8601 format with fractional seconds.
 */
std::string UtcDateUtils::formatCustomTimestampAsIso8601(
    const std::chrono::system_clock::time_point& timestamp) {
    // Convert to time_t
    std::time_t time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::tm time_tm = *std::gmtime(&time_t);

    // Get the fractional seconds
    auto ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp.time_since_epoch()).count();
    int fractional_seconds = ns % 1000000000;

    return formatAsIso8601(time_tm, fractional_seconds);
}

/**
 * @brief Helper function to format a tm structure as an ISO 8601 string with fractional seconds.
 *
 * @param time_struct The tm structure to format.
 * @param fractional_seconds The fractional seconds to include in the ISO 8601 format (nanoseconds).
 * @return The formatted date-time string in ISO 8601 format with fractional seconds.
 */
std::string UtcDateUtils::formatAsIso8601(const std::tm& time_struct, int fractional_seconds) {
    std::ostringstream oss;

    // Format the date-time part
    oss << std::put_time(&time_struct, "%Y-%m-%dT%H:%M:%S");

    // Append fractional seconds
    oss << "." << std::setw(9) << std::setfill('0') << fractional_seconds;

    // Append the 'Z' for UTC
    oss << "Z";

    return oss.str();
}