#ifndef STATUS_MESSAGE_DTO_H
#define STATUS_MESSAGE_DTO_H

#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <string>

struct StatusMessageErrorDTO {
    int code;
    std::string message;
    nlohmann::json data;

    friend std::ostream &operator<<(std::ostream &out_stream, const StatusMessageErrorDTO &error) {
        out_stream << "{\n"
                   << "  code: " << error.code << "\n"
                   << "  message: " << error.message << "\n"
                   << "  data: " << error.data.dump(2) << "\n"
                   << "}";
        return out_stream;
    }
};

/**
 * @brief Data Transfer Object for Websocket Status Messages
 */
struct StatusMessageDTO {
    int id;
    std::optional<StatusMessageErrorDTO> error;

    // Overload the << operator to print the DTO
    friend std::ostream &operator<<(std::ostream &out_stream, const StatusMessageDTO &dto) {
        out_stream << "StatusMessageDTO {\n"
                   << "  id: " << dto.id << "\n"
                   << "  error: ";
        if (dto.error.has_value()) {
            out_stream << *dto.error << "\n";
        } else {
            out_stream << "null\n";
        }
        out_stream << "}";
        return out_stream;
    }
};

#endif  // STATUS_MESSAGE_DTO_H