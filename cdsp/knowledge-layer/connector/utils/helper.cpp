#include "helper.h"

#include <GeographicLib/TransverseMercator.hpp>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "coordinate_transform.h"

// Define the static constant
// TODO: Should be a more generic geographical point?
const Wgs84Coord Helper::ZONE_ORIGIN{11.579144, 48.137416, 0.0};

/**
 * @brief Retrieves the current timestamp formatted as a string.
 *
 * This function returns the current date and time formatted according to the specified format.
 * It can optionally include nanoseconds and use UTC time.
 *
 * @param format The format string to use for formatting the timestamp.
 *               This should be a valid format string for strftime.
 * @param include_nanos A boolean flag indicating whether to include nanos in the
 * output.
 * @param use_utc A boolean flag indicating whether to use UTC time instead of local time.
 * @return A formatted timestamp string representing the current date and time.
 */
std::string Helper::getFormattedTimestampNow(const std::string& format, bool include_nanos,
                                             bool use_utc) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    std::optional<std::string> nanos = std::nullopt;
    if (include_nanos) {
        nanos = extractNanoseconds(now);
    }
    return formatTimeT(use_utc, now_time_t, format, nanos);
}

/**
 * @brief Generates a formatted timestamp string based on the provided format and options.
 *
 * This function converts a given time point into a formatted string representation.
 * The format of the output string is determined by the provided format string.
 * The function can optionally include nanoseconds and use UTC time.
 *
 * @param format The format string to specify the output format of the timestamp.
 *               This should be a valid format string for strftime.
 * @param timestamp The time point to be formatted. This is a time point from
 *                  the system clock.
 * @param include_nanos A boolean flag indicating whether to include
 *                             nanoseconds in the formatted output.
 * @param use_utc A boolean flag indicating whether to use UTC time instead of local time.
 * @return A formatted timestamp string based on the provided format and options.
 */
std::string Helper::getFormattedTimestampCustom(
    const std::string& format, const std::chrono::system_clock::time_point& timestamp,
    bool include_nanos, bool use_utc) {
    std::time_t time_t = std::chrono::system_clock::to_time_t(timestamp);

    std::optional<std::string> nanos = std::nullopt;
    if (include_nanos) {
        nanos = extractNanoseconds(timestamp);
    }

    return formatTimeT(use_utc, time_t, format, nanos);
}

/**
 * @brief Extracts the nanoseconds component from a given time point.
 *
 * This function takes a time point representing a specific point in time
 * and extracts the nanoseconds part of its duration since the epoch.
 * The result is formatted as a string with exactly 9 digits, including
 * leading zeros if necessary.
 *
 * @param timestamp The time point from which to extract the nanoseconds.
 * @return A string representing the nanoseconds component, formatted with
 *         leading zeros to ensure a length of 9 digits.
 */
std::string Helper::extractNanoseconds(const std::chrono::system_clock::time_point& timestamp) {
    auto nanoseconds_count =
        std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp.time_since_epoch()).count();

    uint32_t nanoseconds = nanoseconds_count % 1000000000;  // Ensure only the last 9 digits

    // Format to ensure 9 digits with leading zeros
    std::ostringstream oss;
    oss << std::setw(9) << std::setfill('0') << nanoseconds;

    return oss.str();
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
std::tuple<std::optional<std::tm>, std::optional<int>> Helper::parseISO8601ToTime(
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
 * @brief Converts a `std::chrono::system_clock::time_point` to nanoseconds since the epoch.
 *
 * This function takes a `std::chrono::system_clock::time_point` and converts it to nanoseconds
 * since the epoch. The result is returned as a `std::chrono::nanoseconds` object.
 *
 * @param timestamp The time point to convert to nanoseconds since the epoch.
 * @return std::chrono::nanoseconds The number of nanoseconds since the epoch for the given time.
 */
std::chrono::nanoseconds Helper::getNanosecondsSinceEpoch(
    const std::chrono::system_clock::time_point& timestamp) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(timestamp.time_since_epoch());
}

/**
 * @brief Converts a `std::chrono::system_clock::time_point` to seconds and nanoseconds since
 * epoch.alignas
 *
 * This function takes a `std::chrono::system_clock::time_point` and converts it to a pair of
 * int64_t representing seconds and nanoseconds since the epoch. The result is returned as a
 * `std::pair<int64_t, int64_t>`.
 *
 * @param timestamp The time point to convert to seconds and nanoseconds since the epoch.
 * @return std::pair<int64_t, int64_t> A pair of int64_t representing seconds and nanoseconds since
 * the epoch for the given time.
 */
std::pair<int64_t, int64_t> Helper::getSecondsAndNanosecondsSinceEpoch(
    const std::chrono::system_clock::time_point& timestamp) {
    auto duration = timestamp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    return {seconds.count(), nanoseconds.count()};
}

/**
 * @brief Retrieves the value of an environment variable.
 *
 * This function attempts to retrieve the value of a specified environment variable.
 * If the environment variable is not set, it returns a provided default value.
 *
 * @param env_var The name of the environment variable to retrieve.
 * @param default_value The value to return if the environment variable is not set. Defaults to
 * an empty string.
 * @return std::string The value of the environment variable, or the default value if the
 * variable is not set.
 */
std::string Helper::getEnvVariable(const std::string& env_var,
                                   const std::optional<std::string>& default_value) {
    const char* value_env = std::getenv(env_var.c_str());
    return value_env ? std::string(value_env) : default_value.value_or("");
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
std::optional<NtmCoord> Helper::getCoordInNtm(const std::string& latitude,
                                              const std::string& longitude) {
    if (latitude.empty() || longitude.empty()) {
        return std::nullopt;
    }

    Wgs84Coord coord_to_convert;
    coord_to_convert.latitude = std::stod(latitude);
    coord_to_convert.longitude = std::stod(longitude);

    return CoordinateTransform::ntmPoseFromWgs84(ZONE_ORIGIN, coord_to_convert);
}

/**
 * @brief Converts a given string to lowercase.
 *
 * This function takes an input string and converts all its characters to lowercase.
 *
 * @param input The input string to be converted.
 * @return A new string with all characters in lowercase.
 */
std::string Helper::toLowerCase(const std::string& input) {
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
std::string Helper::toUppercase(const std::string& input) {
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
std::string Helper::trimTrailingNewlines(const std::string& str) {
    std::string trimmed = str;
    while (!trimmed.empty() && trimmed.back() == '\n') {
        trimmed.pop_back();
    }
    return trimmed;
}

/**
 * @brief Detects the type of a given string value for a JSON.
 *
 * This function attempts to detect the type of a given string value by checking
 * if it can be converted to a boolean, integer, or floating-point number. If the
 * value cannot be converted to any of these types, it is returned as a string.
 *
 * @param value The string value to detect the type of.
 * @return nlohmann::json The detected type of the value.
 */
nlohmann::json Helper::detectType(const std::string& value) {
    if (value.empty()) {
        return "";  // Keep empty values as empty strings
    }

    // Check for boolean values
    if (value == "true" || value == "false") {
        return value == "true";
    }

    // Check for integer values
    char* end;
    long int_val = std::strtol(value.c_str(), &end, 10);
    if (*end == '\0') {
        return int_val;
    }

    // Check for floating-point values
    double float_val = std::strtod(value.c_str(), &end);
    if (*end == '\0') {
        return float_val;
    }

    // Default to string
    return value;
}
std::vector<std::string> Helper::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

/**
 * @brief Converts a JSON value to its string representation.
 *
 * This function takes a JSON value and converts it to a string representation
 * based on its type. Supported types include string, float, integer, unsigned integer,
 * and boolean. If the JSON value is of an unsupported type, a runtime error is thrown.
 *
 * @param json_value The JSON value to be converted to a string.
 * @return std::string The string representation of the JSON value.
 * @throws std::runtime_error If the JSON value is of an unsupported type.
 */
std::string Helper::jsonToString(const nlohmann::json& json_value) {
    if (json_value.is_string()) {
        return json_value.get<std::string>();
    } else if (json_value.is_number_float()) {
        return std::to_string(json_value.get<double>());
    } else if (json_value.is_number_integer()) {
        return std::to_string(json_value.get<int>());
    } else if (json_value.is_number_unsigned()) {
        return std::to_string(json_value.get<unsigned>());
    } else if (json_value.is_boolean()) {
        return json_value.get<bool>() ? "true" : "false";
    }
    throw std::runtime_error("The message contains a node with an unsupported value.");
}

/**
 * @brief Converts a std::variant containing different types to a std::string.
 *
 * This function takes a std::variant that can hold a std::string, int, double, float, or bool,
 * and converts the contained value to a std::string. It uses std::visit to apply a lambda
 * function that handles each possible type:
 * - If the value is a std::string, it is returned directly.
 * - If the value is a bool, it is converted to "true" or "false".
 * - If the value is a numeric type (int, double, float), it is converted using std::to_string.
 *
 * @param var A std::variant containing a value of type std::string, int, double, float, or bool.
 *
 * @return A std::string representation of the value contained in the variant.
 */
std::string Helper::variantToString(
    const std::variant<std::string, int, double, float, bool>& var) {
    return std::visit(
        [](const auto& value) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>) {
                return value;  // Return string directly
            } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, bool>) {
                return value ? "true" : "false";  // Convert boolean to string
            } else {
                return std::to_string(value);  // Convert numbers using std::to_string
            }
        },
        var);
}

/**
 * @brief Converts a given number of seconds and nanoseconds into a system clock timestamp.
 *
 * This function takes two parameters representing time in seconds and nanoseconds,
 * and converts them into a `std::chrono::system_clock::time_point` object.
 * If the conversion fails, an invalid_argument exception is thrown with an error message.
 *
 * @param seconds The number of seconds to be converted into a timestamp.
 * @param nanos The number of nanoseconds to be added to the timestamp.
 * @return std::chrono::system_clock::time_point The resulting timestamp as a time_point object.
 * @throws std::invalid_argument If the conversion fails due to an invalid input.
 */
std::chrono::system_clock::time_point Helper::convertToTimestamp(int64_t seconds, int64_t nanos) {
    try {
        return std::chrono::system_clock::time_point(
            std::chrono::duration_cast<std::chrono::system_clock::duration>(
                std::chrono::seconds(seconds) + std::chrono::nanoseconds(nanos)));
    } catch (const std::exception& e) {
        throw std::invalid_argument("Failed to convert timestamp: " + std::string(e.what()));
    }
}

/**
 * @brief Formats a given time as a string according to the specified format.
 *
 * This function converts a `std::time_t` value into a formatted string representation.
 * The format of the output string is determined by the provided format string.
 * The function can optionally include nanoseconds and use UTC time.
 *
 * @param use_utc A boolean flag indicating whether to use UTC time for the
 *                formatted output. If false, local time is used.
 * @param time_t A reference to the `std::time_t` value representing the time to be formatted.
 * @param format The format string to specify the output format of the time.
 *               This string should be compatible with the formatting options
 *               used by the underlying time formatting functions.
 * @param nanos An optional integer representing the nanoseconds to append
 *                    to the formatted time. If not provided, nanoseconds are not included.
 * @return A formatted time string based on the provided format and options.
 */
std::string Helper::formatTimeT(bool use_utc, std::time_t& time_t, const std::string& format,
                                std::optional<std::string> nanos) {
    // Convert time_t to tm (local or UTC)
    auto tm = use_utc ? *std::gmtime(&time_t) : *std::localtime(&time_t);

    // Format the time according to the provided format string
    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());

    if (nanos.has_value()) {
        oss << '.' << nanos.value();
    }

    std::string formatted_time = oss.str();
    if (use_utc && format.find("T") != std::string::npos) {
        formatted_time += 'Z';
    }

    return formatted_time;
}