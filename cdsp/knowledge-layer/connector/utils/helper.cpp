#include "helper.h"

/**
 * @brief Converts a given string to lowercase.
 *
 * This function takes an input string and converts all its characters to lowercase.
 *
 * @param input The input string to be converted.
 * @return A new string with all characters in lowercase.
 */
std::string toLowercase(const std::string& input) {
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
