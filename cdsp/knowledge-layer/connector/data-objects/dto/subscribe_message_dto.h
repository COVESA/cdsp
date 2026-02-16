#ifndef SUBSCRIBE_MESSAGE_DTO_H
#define SUBSCRIBE_MESSAGE_DTO_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "globals.h"

/**
 * @brief Data Transfer Object for SubscribeMessage.
 */
struct SubscribeMessageDTO {
    int id;
    std::string schema;
    std::string instance;
    std::optional<std::string> path;
    std::optional<std::string> format;
    std::optional<std::string> root;
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a SubscribeMessageDTO object into a JSON
 * representation.
 *
 * @param j The JSON object to which the SubscribeMessageDTO will be serialized.
 * @param dto The SubscribeMessageDTO object containing the data to be
 * serialized.
 */
inline void to_json(nlohmann::json &json_obj, const SubscribeMessageDTO &dto) {
    nlohmann::json::object_t params_obj;
    params_obj = nlohmann::json{{"schema", dto.schema}, {"instance", dto.instance}};
    if (dto.path) {
        params_obj["path"] = *dto.path;
    }
    if (dto.format) {
        params_obj["format"] = *dto.format;
    }
    if (dto.root) {
        params_obj["root"] = *dto.root;
    }
    json_obj =
        nlohmann::json{{"jsonrpc", getJsonRpcVersion()}, {"id", dto.id}, {"params", params_obj}};
}

#endif  // SUBSCRIBE_MESSAGE_DTO_H