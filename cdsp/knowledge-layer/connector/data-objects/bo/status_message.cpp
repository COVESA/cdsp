#include "status_message.h"

/**
 * @brief Constructs a StatusMessage object with the given parameters.
 *
 * @param code An integer representing the status code.
 * @param message A string containing the status message. Must not be empty.
 * @param requestId An optional string representing the request ID.
 * @param timestamp A time_point representing the timestamp of the status message. Must not be
 * empty.
 *
 * @throws std::invalid_argument if the message is empty.
 * @throws std::invalid_argument if the timestamp is empty.
 */
StatusMessage::StatusMessage(int code, const std::string& message,
                             const std::optional<std::string>& requestId,
                             const std::chrono::system_clock::time_point& timestamp)
    : code_(code), message_(message), request_id_(requestId), timestamp_(timestamp) {
    if (message_.empty()) {
        throw std::invalid_argument("StatusMessage message cannot be empty");
    }
}

/**
 * @brief Returns the status code.
 *
 * @return An integer representing the status code.
 */
int StatusMessage::getCode() const { return code_; }

/**
 * @brief Returns the status message.
 *
 * @return A string containing the status message.
 */
std::string StatusMessage::getMessage() const { return message_; }

/**
 * @brief Returns the request ID.
 *
 * @return An optional string representing the request ID.
 */
std::optional<std::string> StatusMessage::getRequestId() const { return request_id_; }

/**
 * @brief Returns the timestamp.
 *
 * @return A time_point representing the timestamp of the status message.
 */
std::chrono::system_clock::time_point StatusMessage::getTimestamp() const { return timestamp_; }

/**
 * @brief Overloads the << operator to print the StatusMessage object.
 *
 * @param os The output stream to write to.
 * @param message The StatusMessage object to print.
 * @return std::ostream& The output stream with the StatusMessage object printed to it.
 */
std::ostream& operator<<(std::ostream& os, const StatusMessage& message) {
    os << "StatusMessage {" << "\n";
    os << "  code: " << message.getCode() << ",\n";
    os << "  message: " << message.getMessage() << ",\n";
    os << "  request_id: "
       << (message.getRequestId() ? *message.getRequestId() : std::string("null")) << ",\n";
    os << "  timestamp: " << message.getTimestamp().time_since_epoch().count() << " nanos\n";
    os << "}";

    return os;
}