#ifndef DATA_MESSAGE_DTO_H
#define DATA_MESSAGE_DTO_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "metadata_dto.h"
/**
 * @brief Data Transfer Object for Websocket Messages
 */
struct DataMessageDto {
    std::string type;
    std::string schema;
    std::string instance;
    std::optional<std::string> path;
    nlohmann::json data;
    std::optional<MetadataDto> metadata;
    std::optional<std::string> requestId;

    // Overload the << operator to print the DTO
    friend std::ostream& operator<<(std::ostream& os, const DataMessageDto& dto) {
        os << "DataMessageDto {\n"
           << "  type: " << dto.type << "\n"
           << "  schema: " << dto.schema << "\n"
           << "  instance: " << dto.instance << "\n"
           << "  data: " << dto.data.dump(4) << "\n"
           << "  path: " << (dto.path ? *dto.path : "null") << "\n"
           << "  metadata: ";

        if (dto.metadata) {
            os << dto.metadata.value();
        } else {
            os << "null";
        }

        os << "\n  requestId: " << (dto.requestId ? *dto.requestId : "null") << "\n"
           << "}";
        return os;
    }
};

#endif  // DATA_MESSAGE_DTO_H