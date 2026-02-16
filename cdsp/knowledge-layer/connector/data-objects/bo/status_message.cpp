#include "status_message.h"

/**
 * @brief Constructs a StatusMessage object with the given parameters.
 *
 * @param identifier A string representing the request ID.
 * @param error An optional Error object representing the error associated with
 * the status message.
 */
StatusMessage::StatusMessage(int identifier, std::optional<Error> error)
    : id_(identifier), error_(std::move(error)) {
    if (id_ < 0) {
        throw std::invalid_argument("StatusMessage ID cannot be negative");
    }
}

/**
 * @brief Returns the ID of the request.
 *
 * @return An integer representing the request ID.
 */
int StatusMessage::getIdentifier() const { return id_; }

/**
 * @brief Returns the optional Error object associated with the status message.
 *
 * @return An optional Error object.
 */
std::optional<Error> StatusMessage::getError() const { return error_; }

/**
 * @brief Overloads the << operator to print the StatusMessage object.
 *
 * @param os The output stream to write to.
 * @param message The StatusMessage object to print.
 * @return std::ostream& The output stream with the StatusMessage object printed
 * to it.
 */
std::ostream &operator<<(std::ostream &out_stream, const StatusMessage &message) {
    out_stream << "StatusMessage {" << "\n";
    out_stream << "  id: " << message.getIdentifier() << ",\n";
    if (message.getError()) {
        out_stream << "  error: " << *message.getError() << ",\n";
    } else {
        out_stream << "  error: null,\n";
    }
    out_stream << "}";

    return out_stream;
}