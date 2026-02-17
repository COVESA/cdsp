#ifndef UNSUBSCRIBE_MESSAGE_DTO
#define UNSUBSCRIBE_MESSAGE_DTO

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "globals.h"

/**
 * @brief Data Transfer Object for UnsubscribeMessage.
 */
struct UnsubscribeMessageDTO {
    int id;
    std::string schema;
    std::string instance;
    std::optional<std::string> path;
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a UnsubscribeMessageDTO object into a JSON
 * representation.
 *
 * @param json_obj The JSON object to which the UnsubscribeMessageDTO will be
 * serialized.
 * @param dto The UnsubscribeMessageDTO object containing the data to be
 * serialized.
 */
inline void to_json(nlohmann::json &json_obj, const UnsubscribeMessageDTO &dto) {
    nlohmann::json::object_t params_obj;
    params_obj = nlohmann::json{{"schema", dto.schema}, {"instance", dto.instance}};
    if (dto.path) {
        params_obj["path"] = *dto.path;
    }
    json_obj = nlohmann::json{{"jsonrpc", getJsonRpcVersion()},
                              {"method", "unsubscribe"},
                              {"id", dto.id},
                              {"params", params_obj}};
}

#endif  // UNSUBSCRIBE_MESSAGE_DTO