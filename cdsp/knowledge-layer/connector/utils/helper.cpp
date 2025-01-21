#include "helper.h"

#include <GeographicLib/TransverseMercator.hpp>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "coordinate_transform.h"

// Define the private namespace for internal helper functions
namespace {
std::tm getCurrentTime(bool use_utc) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    return use_utc ? *std::gmtime(&now_time_t) : *std::localtime(&now_time_t);
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

std::string addMilliseconds(const std::string& formatted_time, int milliseconds) {
    std::ostringstream oss;
    oss << formatted_time << '.' << std::setw(3) << std::setfill('0') << milliseconds;
    return oss.str();
}
}  // namespace

// Public functions
namespace Helper {
/**
 * @brief Generates a formatted timestamp string.
 *
 * This function creates a formatted timestamp string based on the provided format.
 * It can include milliseconds and use UTC time if specified. Optionally, a custom
 * time and milliseconds can be provided; otherwise, the current time is used.
 *
 * @param format The format string for the timestamp.
 * @param include_milliseconds Boolean flag to include milliseconds in the output.
 * @param use_utc Boolean flag to use UTC time instead of local time.
 * @param custom_time Optional custom time to use instead of the current time.
 * @param custom_milliseconds Optional custom milliseconds to use instead of the current milliseconds.
 * @return std::string The formatted timestamp string.
 */
std::string getFormattedTimestamp(const std::string& format, bool include_milliseconds, bool use_utc,
                                  const std::optional<std::tm>& custom_time,
                                  const std::optional<int>& custom_milliseconds) {
    std::tm time = custom_time.has_value() ? custom_time.value() : getCurrentTime(use_utc);
    int milliseconds = custom_milliseconds.has_value()
                           ? custom_milliseconds.value()
                           : (include_milliseconds ? getCurrentMilliseconds() : 0);

    std::string formatted_time = formatTime(time, format);

    if (include_milliseconds) {
        formatted_time = addMilliseconds(formatted_time, milliseconds);
    }

    if (use_utc && format.find("T") != std::string::npos) {
        formatted_time += 'Z';
    }

    return formatted_time;
}

/**
 * @brief Converts latitude and longitude strings to NTM coordinates.
 *
 * This function takes latitude and longitude as strings, converts them to
 * a WGS84 coordinate, and then transforms it into NTM coordinates using
 * the `CoordinateTransform::ntmPoseFromWgs84` function. If either input
 * string is empty, it returns an empty optional.
 *
 * @param latitude The latitude as a string.
 * @param longitude The longitude as a string.
 * @return std::optional<NtmCoord> The converted NTM coordinates wrapped in
 *         an optional. If the conversion fails or inputs are empty, returns
 *         an empty optional.
 */
std::optional<NtmCoord> getCoordInNtm(const std::string& latitude, const std::string& longitude) {
    if (latitude.empty() || longitude.empty()) {
        return std::nullopt;
    }

    Wgs84Coord coord_to_convert;
    coord_to_convert.latitude = std::stod(latitude);
    coord_to_convert.longitude = std::stod(longitude);

    return CoordinateTransform::ntmPoseFromWgs84(Helper::ZONE_ORIGIN, coord_to_convert);
}

/**
 * @brief Parses an ISO 8601 formatted datetime string into a `std::tm` structure and extracts
 * milliseconds.
 *
 * This function takes a datetime string in ISO 8601 format (e.g., "2024-11-12T07:45:34.404Z")
 * and parses it into a `std::tm` structure representing the date and time up to seconds.
 * If milliseconds are present in the string, they are also extracted as an integer.
 *
 * @param iso_string The ISO 8601 formatted datetime string to parse.
 *                  Expected format: "YYYY-MM-DDTHH:MM:SS[.sss]Z"
 *                  Milliseconds (".sss") are optional.
 * @return A tuple containing:
 *         - `std::optional<std::tm>`: The parsed date and time as a `std::tm` structure.
 *           If parsing fails, this will be `std::nullopt`.
 *         - `std::optional<int>`: The milliseconds as an integer, if available. If no
 *           milliseconds are present in the string, this will be `std::nullopt`.
 */
std::tuple<std::optional<std::tm>, std::optional<int>> parseISO8601ToTime(
    const std::string& iso_string) {
    std::tm tm = {};
    std::optional<int> milliseconds = std::nullopt;

    // Parse the date and time components up to seconds
    std::istringstream iss(iso_string);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (iss.fail()) {
        std::cout << "Failed to parse datetime" << std::endl;
        return {std::nullopt, std::nullopt};
    }

    // Check for milliseconds in the format .sss
    if (iso_string.find('.') != std::string::npos) {
        std::string millisStr = iso_string.substr(iso_string.find('.') + 1, 3);
        milliseconds = std::stoi(millisStr);
    }

    return {tm, milliseconds};
}

/**
 * @brief Converts an ISO 8601 formatted string to the duration in milliseconds since the epoch.
 *
 * This function takes an ISO 8601 formatted date-time string and calculates the duration in
 * milliseconds since the Unix epoch (January 1, 1970). It uses a helper function
 * `parseISO8601ToTime` to parse the string into a `tm` structure and milliseconds.
 *
 * @param iso_string The ISO 8601 formatted date-time string to be converted.
 * @return std::chrono::duration<double, std::milli> The duration in milliseconds since the epoch.
 *         If the input string cannot be parsed, returns a duration of 0 milliseconds.
 */
std::chrono::duration<double, std::milli> getMillisecondsSinceEpoch(const std::string& iso_string) {
    auto [tm, milliseconds] = parseISO8601ToTime(iso_string);
    if (!tm.has_value()) {
        return std::chrono::milliseconds(0);
    }
    if (!milliseconds.has_value()) {
        milliseconds = 0;
    }

    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm.value())) +
              std::chrono::milliseconds(milliseconds.value());

    auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
        tp.time_since_epoch());
    return duration;
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