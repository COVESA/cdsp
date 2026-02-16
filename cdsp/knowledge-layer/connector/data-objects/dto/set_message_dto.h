#ifndef SET_MESSAGE_DTO_H
#define SET_MESSAGE_DTO_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "globals.h"
#include "metadata_dto.h"

/**
 * @brief Data Transfer Object for Data.
 */
struct DataDTO {
    std::string name;
    nlohmann::json value;

    // Overload the << operator to print the DataDTO
    friend std::ostream &operator<<(std::ostream &out_stream, const DataDTO &dto) {
        out_stream << "    DataDTO {\n"
                   << "      name: " << dto.name << "\n"
                   << "      value: " << dto.value.dump() << "\n"
                   << "    }";
        return out_stream;
    }
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a DataDTO object into a JSON representation.
 *
 * @param json_obj The JSON object to which the DataDTO will be serialized.
 * @param dto The DataDTO object containing the data to be serialized.
 */
inline void to_json(nlohmann::json &json_obj, const DataDTO &dto) {
    json_obj = nlohmann::json{{dto.name, dto.value}};
}
/**
 * @brief Data Transfer Object for SetMessage.
 */
struct SetMessageDTO {
    int id;
    std::string schema;
    std::string instance;
    std::vector<DataDTO> data;
    std::optional<std::string> path;
    MetadataDTO metadata;

    // Overload the << operator to print the SetMessageDTO
    friend std::ostream &operator<<(std::ostream &out_stream, const SetMessageDTO &dto) {
        out_stream << "SetMessageDTO {\n"
                   << "  id: " << dto.id << "\n"
                   << "  schema: " << dto.schema << "\n"
                   << "  instance: " << dto.instance << "\n"
                   << "  path: " << (dto.path ? *dto.path : "null") << "\n"
                   << "  data: [\n";
        for (const auto &data : dto.data) {
            out_stream << data << ",\n";
        }
        out_stream << "  ]\n";
        out_stream << "  metadata: " << dto.metadata << "\n";
        out_stream << "}";
        return out_stream;
    }
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a SetMessageDTO object into a JSON representation.
 *
 * @param j The JSON object to which the SetMessageDTO will be serialized.
 * @param dto The SetMessageDTO object containing the data to be serialized.
 */
inline void to_json(nlohmann::json &json_obj, const SetMessageDTO &dto) {
    nlohmann::json data_obj = nlohmann::json::object();
    for (const auto &item : dto.data) {
        data_obj[item.name] = item.value;
    }

    nlohmann::json params_obj = nlohmann::json::object();
    params_obj = nlohmann::json{{"schema", dto.schema},
                                {"instance", dto.instance},
                                {"data", data_obj},
                                {"metadata", dto.metadata}};
    if (dto.path) {
        params_obj["path"] = *dto.path;
    }

    json_obj = nlohmann::json{{"jsonrpc", getJsonRpcVersion()},
                              {"method", "set"},
                              {"id", dto.id},
                              {"params", params_obj}};
}

#endif  // SET_MESSAGE_DTO_H