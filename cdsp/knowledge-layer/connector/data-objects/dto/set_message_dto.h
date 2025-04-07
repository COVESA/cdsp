#ifndef SET_MESSAGE_DTO_H
#define SET_MESSAGE_DTO_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "metadata_dto.h"

/**
 * @brief Data Transfer Object for Data.
 */
struct DataDTO {
    std::string name;
    nlohmann::json value;

    // Overload the << operator to print the DataDTO
    friend std::ostream& operator<<(std::ostream& os, const DataDTO& dto) {
        os << "    DataDTO {\n"
           << "      name: " << dto.name << "\n"
           << "      value: " << dto.value.dump() << "\n"
           << "    }";
        return os;
    }
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a DataDTO object into a JSON representation.
 *
 * @param j The JSON object to which the DataDTO will be serialized.
 * @param dto The DataDTO object containing the data to be serialized.
 */
inline void to_json(nlohmann::json& j, const DataDTO& dto) {
    j = nlohmann::json{{dto.name, dto.value}};
}

/**
 * @brief Data Transfer Object for SetMessage.
 */
struct SetMessageDTO {
    std::string schema;
    std::string instance;
    std::vector<DataDTO> data;
    std::optional<std::string> path;
    std::optional<std::string> requestId;
    MetadataDTO metadata;

    // Overload the << operator to print the SetMessageDTO
    friend std::ostream& operator<<(std::ostream& os, const SetMessageDTO& dto) {
        os << "SetMessageDTO {\n"
           << "  schema: " << dto.schema << "\n"
           << "  instance: " << dto.instance << "\n"
           << "  path: " << (dto.path ? *dto.path : "null") << "\n"
           << "  requestId: " << (dto.requestId ? *dto.requestId : "null") << "\n"
           << "  data: [\n";
        for (const auto& data : dto.data) {
            os << data << ",\n";
        }
        os << "  ]\n";
        os << "  metadata: " << dto.metadata << "\n";
        os << "}";
        return os;
    }
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a SetMessageDTO object into a JSON representation.
 * It includes mandatory fields such as schema, instance, and data, and conditionally
 * includes optional fields if they are present.
 *
 * @param j The JSON object to which the SetMessageDTO will be serialized.
 * @param dto The SetMessageDTO object containing the data to be serialized.
 */
inline void to_json(nlohmann::json& j, const SetMessageDTO& dto) {
    nlohmann::json data_obj = nlohmann::json::object();
    for (const auto& item : dto.data) {
        data_obj[item.name] = item.value;
    }

    j = nlohmann::json{{"type", "set"},
                       {"schema", dto.schema},
                       {"instance", dto.instance},
                       {"data", data_obj},
                       {"metadata", dto.metadata}};
    if (dto.path)
        j["path"] = *dto.path;
    if (dto.requestId)
        j["requestId"] = *dto.requestId;
}

#endif  // SET_MESSAGE_DTO_H