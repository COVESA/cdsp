#ifndef DATA_MESSAGE_DTO_H
#define DATA_MESSAGE_DTO_H

#include <nlohmann/json.hpp>
#include <optional>

#include "metadata_dto.h"
/**
 * @brief Data Transfer Object for Websocket Messages
 */
struct DataMessageDTO {
    int id;
    nlohmann::json data;
    std::optional<MetadataDTO> metadata;

    // Overload the << operator to print the DTO
    friend std::ostream &operator<<(std::ostream &out_stream, const DataMessageDTO &dto) {
        out_stream << "DataMessageDTO {\n"
                   << "  id: " << dto.id << "\n"
                   << "  data: " << dto.data.dump(4) << "\n"
                   << "  metadata: ";

        if (dto.metadata) {
            out_stream << dto.metadata.value();
        } else {
            out_stream << "null";
        }

        out_stream << "\n"
                   << "}\n";
        return out_stream;
    }
};

#endif  // DATA_MESSAGE_DTO_H