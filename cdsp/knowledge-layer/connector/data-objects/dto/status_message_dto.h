#ifndef STATUS_MESSAGE_DTO_H
#define STATUS_MESSAGE_DTO_H

#include <optional>
#include <string>

/**
 * @brief Data Transfer Object for Websocket Status Messages
 */
struct StatusMessageDTO {
    int code;
    std::string message;
    std::optional<std::string> requestId;
    struct {
        int seconds;
        int nanos;
    } timestamp;

    // Overload the << operator to print the DTO
    friend std::ostream& operator<<(std::ostream& os, const StatusMessageDTO& dto) {
        os << "StatusMessageDTO {\n"
           << "  code: " << dto.code << "\n"
           << "  message: " << dto.message << "\n"
           << "  requestId: " << (dto.requestId ? *dto.requestId : "null") << "\n"
           << "  timestamp: { seconds: " << dto.timestamp.seconds
           << ", nanos: " << dto.timestamp.nanos << " }\n"
           << "}";
        return os;
    }
};

#endif  // STATUS_MESSAGE_DTO_H