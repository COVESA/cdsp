#include "helper.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

// Define the private namespace for internal helper functions
namespace {
std::tm getCurrentTime(bool useUTC) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    return useUTC ? *std::gmtime(&now_time_t) : *std::localtime(&now_time_t);
}

int getCurrentMilliseconds() {
    auto now = std::chrono::system_clock::now();
    return (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000)
        .count();
}

std::string formatTime(const std::tm& time, const std::string& format) {
    std::ostringstream oss;
    oss << std::put_time(&time, format.c_str());
    return oss.str();
}

std::string addMilliseconds(const std::string& formattedTime, int milliseconds) {
    std::ostringstream oss;
    oss << formattedTime << '.' << std::setw(3) << std::setfill('0') << milliseconds;
    return oss.str();
}
}  // namespace

// Public functions
namespace Helper {
/**
 * @brief Formats a timestamp based on the provided time, format, and optional milliseconds.
 *
 * This function generates a formatted timestamp string based on the provided time components.
 * If a time structure (`std::tm`) is provided, it formats it directly. Otherwise, it uses the
 * current time. The function can include milliseconds and append a 'Z' to indicate UTC if
 * specified.
 *
 * @param format The desired format of the timestamp string (e.g., "%Y-%m-%dT%H:%M:%S").
 * @param includeMilliseconds A boolean indicating whether milliseconds should be appended to the
 * timestamp.
 * @param useUTC A boolean indicating whether the timestamp should be in UTC format (adds 'Z' to the
 * end if `T` is in the format).
 * @param tm An optional `std::tm` structure containing the time to format. If not provided, the
 * current time is used.
 * @param milliseconds An optional integer representing milliseconds to include. Ignored if
 * `includeMilliseconds` is false.
 * @return A string containing the formatted timestamp.
 */
std::string getFormattedTimestamp(const std::string& format, bool includeMilliseconds, bool useUTC,
                                  const std::optional<std::tm>& customTime,
                                  const std::optional<int>& customMilliseconds) {
    std::tm time = customTime.has_value() ? customTime.value() : getCurrentTime(useUTC);
    int milliseconds = customMilliseconds.has_value()
                           ? customMilliseconds.value()
                           : (includeMilliseconds ? getCurrentMilliseconds() : 0);

    std::string formattedTime = formatTime(time, format);

    if (includeMilliseconds) {
        formattedTime = addMilliseconds(formattedTime, milliseconds);
    }

    if (useUTC && format.find("T") != std::string::npos) {
        formattedTime += 'Z';
    }

    return formattedTime;
}

/**
 * @brief Parses an ISO 8601 formatted datetime string into a `std::tm` structure and extracts
 * milliseconds.
 *
 * This function takes a datetime string in ISO 8601 format (e.g., "2024-11-12T07:45:34.404Z")
 * and parses it into a `std::tm` structure representing the date and time up to seconds.
 * If milliseconds are present in the string, they are also extracted as an integer.
 *
 * @param isoString The ISO 8601 formatted datetime string to parse.
 *                  Expected format: "YYYY-MM-DDTHH:MM:SS[.sss]Z"
 *                  Milliseconds (".sss") are optional.
 * @return A tuple containing:
 *         - `std::optional<std::tm>`: The parsed date and time as a `std::tm` structure.
 *           If parsing fails, this will be `std::nullopt`.
 *         - `std::optional<int>`: The milliseconds as an integer, if available. If no
 *           milliseconds are present in the string, this will be `std::nullopt`.
 */
std::tuple<std::optional<std::tm>, std::optional<int>> parseISO8601ToTime(
    const std::string& isoString) {
    std::tm tm = {};
    std::optional<int> milliseconds = std::nullopt;

    // Parse the date and time components up to seconds
    std::istringstream iss(isoString);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (iss.fail()) {
        std::cout << "Failed to parse datetime" << std::endl;
        return {std::nullopt, std::nullopt};
    }

    // Check for milliseconds in the format .sss
    if (isoString.find('.') != std::string::npos) {
        std::string millisStr = isoString.substr(isoString.find('.') + 1, 3);
        milliseconds = std::stoi(millisStr);
    }

    return {tm, milliseconds};
}

/**
 * @brief Converts a given string to lowercase.
 *
 * This function takes an input string and converts all its characters to lowercase.
 *
 * @param input The input string to be converted.
 * @return A new string with all characters in lowercase.
 */
std::string toLowerCase(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * @brief Converts a given string to uppercase.
 *
 * This function takes an input string and converts all its characters to uppercase.
 *
 * @param input The input string to be converted.
 * @return A new string with all characters in uppercase.
 */
std::string toUppercase(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

/**
 * @brief Removes trailing newline characters from a string.
 *
 * This function iterates over the input string from the end and removes
 * any newline characters ('\n') it encounters until a non-newline character
 * is found or the string becomes empty.
 *
 * @param str The input string from which trailing newlines are to be removed.
 * @return A new string with trailing newline characters removed.
 */
std::string trimTrailingNewlines(const std::string& str) {
    std::string trimmed = str;
    while (!trimmed.empty() && trimmed.back() == '\n') {
        trimmed.pop_back();
    }
    return trimmed;
}
}  // namespace Helper