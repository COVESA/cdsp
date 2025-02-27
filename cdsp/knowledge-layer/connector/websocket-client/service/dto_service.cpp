#include "dto_service.h"

#include <iostream>

#include "metadata_dto.h"

/**
 * Parses a JSON object into a DataMessageDto object.
 *
 * @param json The JSON object to parse, expected to contain keys: "type", "schema", "instance",
 *             and optionally "path", "metadata", and "requestId".
 * @return A DataMessageDto object populated with data extracted from the JSON object.
 */
DataMessageDto DtoService::parseDataDto(const nlohmann::json& json) {
    try {
        if (!json.contains("type") || !json.contains("schema") || !json.contains("instance") ||
            !json.contains("data")) {
            throw std::invalid_argument("Missing required fields in DataMessageDto");
        }

        DataMessageDto dto;
        dto.type = json["type"];
        dto.schema = json["schema"];
        dto.instance = json["instance"];
        dto.path = json.contains("path")
                       ? std::optional<std::string>(json["path"].get<std::string>())
                       : std::nullopt;
        dto.data = json["data"];
        dto.metadata = json.contains("metadata")
                           ? std::optional<MetadataDto>(parseMetadataDto(json["metadata"]))
                           : std::nullopt;

        dto.requestId = json.contains("requestId")
                            ? std::optional<std::string>(json["requestId"].get<std::string>())
                            : std::nullopt;
        if (!dto.path.has_value() && !dto.data.is_object()) {
            throw std::invalid_argument("DataMessageDto must have a path if data is not an object");
        }
        return dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("Failed to parse DataMessageDto: " + std::string(e.what()));
    }
}

/**
 * Parses a JSON object into a StatusMessageDto.
 *
 * @param json The JSON object containing the status message data.
 * @return A StatusMessageDto object populated with data from the JSON object.
 */
StatusMessageDto DtoService::parseStatusDto(const nlohmann::json& json) {
    try {
        if (!json.contains("code") || !json.contains("message") || !json.contains("timestamp") ||
            !json["timestamp"].contains("seconds") || !json["timestamp"].contains("nanoseconds")) {
            throw std::invalid_argument("Missing required fields in StatusMessageDto");
        }
        StatusMessageDto dto;
        dto.code = json["code"];
        dto.message = json["message"];
        dto.requestId = json.contains("requestId")
                            ? std::optional<std::string>(json["requestId"].get<std::string>())
                            : std::nullopt;

        dto.timestamp.seconds = json["timestamp"]["seconds"];
        dto.timestamp.nanoseconds = json["timestamp"]["nanoseconds"];
        return dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("Failed to parse StatusMessageDto: " + std::string(e.what()));
    }
}

/**
 * Parses a JSON object containing timestamps into a MetadataDto object.
 *
 * @param metadata_json The JSON object containing the metadata data.
 * @return A MetadataDto object populated with data from the JSON object.
 */
MetadataDto DtoService::parseMetadataDto(const nlohmann::json& metadata_json) {
    MetadataDto dto;
    try {
        for (const auto& [node, timestamps] : metadata_json.items()) {
            MetadataDto::NodeMetadata node_metadata;

            // Check and parse received timestamps
            if (timestamps.contains("timestamps") &&
                timestamps["timestamps"].contains("received")) {
                if (!timestamps["timestamps"]["received"].contains("seconds") ||
                    !timestamps["timestamps"]["received"].contains("nanoseconds")) {
                    throw std::invalid_argument("Missing required fields in MetadataDto");
                }
                node_metadata.received.seconds = timestamps["timestamps"]["received"]["seconds"];
                node_metadata.received.nanoseconds =
                    timestamps["timestamps"]["received"]["nanoseconds"];
            }

            // Check and parse generated timestamps
            if (timestamps.contains("timestamps") &&
                timestamps["timestamps"].contains("generated")) {
                if (!timestamps["timestamps"]["generated"].contains("seconds") ||
                    !timestamps["timestamps"]["generated"].contains("nanoseconds")) {
                    throw std::invalid_argument("Missing required fields in MetadataDto");
                }
                node_metadata.generated.seconds = timestamps["timestamps"]["generated"]["seconds"];
                node_metadata.generated.nanoseconds =
                    timestamps["timestamps"]["generated"]["nanoseconds"];
            }
            dto.nodes[node] = node_metadata;
        }
        return dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("Failed to parse MetadataDto: " + std::string(e.what()));
    }
}