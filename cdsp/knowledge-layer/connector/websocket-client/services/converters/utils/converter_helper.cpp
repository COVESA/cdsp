#include "converter_helper.h"

#include <iostream>

#include "helper.h"

/**
 * Parses the given seconds and nanoseconds into a system clock time point.
 *
 * This function attempts to create a std::chrono::system_clock::time_point from the provided
 * seconds and nanoseconds. If an exception is thrown during the conversion, it catches the
 * exception, logs an error message, and returns std::nullopt.
 *
 * @param seconds The number of seconds since the epoch.
 * @param nanos The number of nanoseconds to add to the seconds.
 * @return An optional containing the parsed time point if successful, or std::nullopt if an error
 * occurs.
 */
std::optional<std::chrono::system_clock::time_point> ConverterHelper::parseTimestamp(
    int64_t seconds, int64_t nanos) {
    try {
        if (seconds <= 0 || nanos < 0 || nanos >= 1'000'000'000) {
            return std::nullopt;
        }

        return Helper::convertToTimestamp(seconds, nanos);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse timestamp: " << e.what() << std::endl;
        return std::nullopt;
    }
}