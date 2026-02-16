#include "error.h"

/**
 * @brief Constructs an Error object with the given parameters.
 *
 * @param code An integer representing the error code.
 * @param message A string containing the error message. Must not be empty.
 * @param data An optional JSON object representing additional data for the
 * error.
 *
 * @throws std::invalid_argument if the message is empty.
 */
Error::Error(int code, std::string message, std::optional<nlohmann::json> data)
    : code_(code), message_(std::move(message)), data_(std::move(data)) {
    if (message_.empty()) {
        throw std::invalid_argument("Error message cannot be empty");
    }
}

/**
 * @brief Retrieves the error code.
 *
 * This function returns the error code associated with the Error object.
 * It is a constant member function, meaning it does not modify the state
 * of the object.
 *
 * @return int The error code.
 */
int Error::getCode() const { return code_; }

/**
 * @brief Retrieves the error message.
 *
 * @return A constant reference to the error message string.
 */
const std::string &Error::getMessage() const { return message_; }

/**
 * @brief Retrieves the data for the error.
 *
 * This function returns an optional JSON object that contains the data
 * for the error if it is available.
 *
 * @return A constant reference to an optional JSON object containing the error
 * data.
 */
const std::optional<nlohmann::json> &Error::getData() const { return data_; }

/**
 * @brief Overloads the << operator to print the Error object.
 *
 * This function allows the Error object to be printed using standard output
 * streams. It formats the output to include the error code, message, and data
 * if available.
 *
 * @param os The output stream to write to.
 * @param error The Error object to print.
 * @return std::ostream& The output stream with the Error object printed to it.
 */
std::ostream &operator<<(std::ostream &out_stream, const Error &error) {
    out_stream << "{\n";
    out_stream << "  code: " << error.getCode() << ",\n";
    out_stream << "  message: " << error.getMessage() << ",\n";
    if (error.getData()) {
        out_stream << "  data: " << *error.getData() << ",\n";
    }
    out_stream << "}";

    return out_stream;
}