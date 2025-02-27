#ifndef SUBSCRIBE_MESSAGE_DTO_H
#define SUBSCRIBE_MESSAGE_DTO_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

/**
 * @brief Data Transfer Object for SubscribeMessage.
 */
struct SubscribeMessageDto {
    std::string type;
    std::string schema;
    std::string instance;
    std::optional<std::string> path;
    std::optional<std::string> requestId;
    std::optional<std::string> format;
    std::optional<std::string> root;
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a SubscribeMessageDto object into a JSON representation.
 * It includes mandatory fields such as type, schema, and instance, and conditionally
 * includes optional fields if they are present.
 *
 * @param j The JSON object to which the SubscribeMessageDto will be serialized.
 * @param dto The SubscribeMessageDto object containing the data to be serialized.
 */
inline void to_json(nlohmann::json& j, const SubscribeMessageDto& dto) {
    j = nlohmann::json{{"type", dto.type}, {"schema", dto.schema}, {"instance", dto.instance}};
    if (dto.path)
        j["path"] = *dto.path;
    if (dto.requestId)
        j["requestId"] = *dto.requestId;
    if (dto.format)
        j["format"] = *dto.format;
    if (dto.root)
        j["root"] = *dto.root;
}

#endif  // SUBSCRIBE_MESSAGE_DTO_H