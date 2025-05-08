#include "dto_service.h"

#include <iostream>

#include "helper.h"
#include "metadata_dto.h"

/**
 * Parses a JSON object into a DataMessageDTO object.
 *
 * @param json The JSON object to parse, expected to contain keys: "type", "schema", "instance",
 *             and optionally "path", "metadata", and "requestId".
 * @return A DataMessageDTO object populated with data extracted from the JSON object.
 */
DataMessageDTO DtoService::parseDataJsonToDto(const nlohmann::json& json) {
    try {
        if (!json.contains("type") || !json.contains("schema") || !json.contains("instance") ||
            !json.contains("data")) {
            throw std::invalid_argument("Missing required fields in DataMessageDTO");
        }

        DataMessageDTO dto;
        dto.type = json["type"];
        dto.schema = json["schema"];
        dto.instance = json["instance"];
        dto.path = json.contains("path")
                       ? std::optional<std::string>(json["path"].get<std::string>())
                       : std::nullopt;
        dto.data = json["data"];
        dto.metadata = json.contains("metadata")
                           ? std::optional<MetadataDTO>(parseMetadataJsonToDto(json["metadata"]))
                           : std::nullopt;

        dto.requestId = json.contains("requestId")
                            ? std::optional<std::string>(json["requestId"].get<std::string>())
                            : std::nullopt;
        if (!dto.path.has_value() && !dto.data.is_object()) {
            throw std::invalid_argument("DataMessageDTO must have a path if data is not an object");
        }
        return dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("Failed to parse DataMessageDTO: " + std::string(e.what()));
    }
}

/**
 * Parses a JSON object into a StatusMessageDTO.
 *
 * @param json The JSON object containing the status message data.
 * @return A StatusMessageDTO object populated with data from the JSON object.
 */
StatusMessageDTO DtoService::parseStatusJsonToDto(const nlohmann::json& json) {
    try {
        if (!json.contains("code") || !json.contains("message") || !json.contains("timestamp") ||
            !json["timestamp"].contains("seconds") || !json["timestamp"].contains("nanos")) {
            throw std::invalid_argument("Missing required fields in StatusMessageDTO");
        }
        StatusMessageDTO dto;
        dto.code = json["code"];
        dto.message = json["message"];
        dto.requestId = json.contains("requestId")
                            ? std::optional<std::string>(json["requestId"].get<std::string>())
                            : std::nullopt;

        dto.timestamp.seconds = json["timestamp"]["seconds"];
        dto.timestamp.nanos = json["timestamp"]["nanos"];
        return dto;
    } catch (const std::exception& e) {
        std::cout << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                  << " Status message received:\n"
                  << json.dump() << std::endl
                  << std::endl;
        std::cerr << " Failed to parse StatusMessageDTO: " << e.what() << std::endl;
        return StatusMessageDTO();
    }
}

/**
 * Parses a JSON object containing timestamps into a MetadataDTO object.
 *
 * @param metadata_json The JSON object containing the metadata data.
 * @return A MetadataDTO object populated with data from the JSON object.
 */
MetadataDTO DtoService::parseMetadataJsonToDto(const nlohmann::json& metadata_json) {
    MetadataDTO dto;
    try {
        for (const auto& [node, timestamps] : metadata_json.items()) {
            MetadataDTO::NodeMetadata node_metadata;

            // Check and parse received timestamps
            if (timestamps.contains("timestamps") &&
                timestamps["timestamps"].contains("received")) {
                if (!timestamps["timestamps"]["received"].contains("seconds") ||
                    !timestamps["timestamps"]["received"].contains("nanos")) {
                    throw std::invalid_argument("Missing required fields in MetadataDTO");
                }
                node_metadata.received.seconds = timestamps["timestamps"]["received"]["seconds"];
                node_metadata.received.nanos = timestamps["timestamps"]["received"]["nanos"];
            }

            // Check and parse generated timestamps
            if (timestamps.contains("timestamps") &&
                timestamps["timestamps"].contains("generated")) {
                if (!timestamps["timestamps"]["generated"].contains("seconds") ||
                    !timestamps["timestamps"]["generated"].contains("nanos")) {
                    throw std::invalid_argument("Missing required fields in MetadataDTO");
                }
                node_metadata.generated.seconds = timestamps["timestamps"]["generated"]["seconds"];
                node_metadata.generated.nanos = timestamps["timestamps"]["generated"]["nanos"];
            }
            dto.nodes[node] = node_metadata;
        }
        return dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("Failed to parse MetadataDTO: " + std::string(e.what()));
    }
}

/**
 * Parses a JSON object containing reasoning model configuration into a ModelConfigDTO.
 *
 * @param json The JSON object containing the model configuration data.
 * @return A ModelConfigDTO object populated with data from the JSON object.
 */
ModelConfigDTO DtoService::parseModelConfigJsonToDto(const nlohmann::json& json) {
    try {
        if (!json.contains("inputs") || !json.contains("ontologies") || !json.contains("output") ||
            !json.contains("rules") || !json.contains("shacl") || !json.contains("queries") ||
            !json.contains("reasoner_settings")) {
            throw std::invalid_argument("Missing required fields in ModelConfigDTO");
        }
        QueriesDTO queries_dto;
        ReasonerSettingsDTO reasoner_settings_dto;
        ModelConfigDTO model_config_dto;

        model_config_dto.inputs = json["inputs"];
        model_config_dto.ontologies = json["ontologies"];
        model_config_dto.output = json["output"];
        model_config_dto.rules = json["rules"];
        model_config_dto.shacl_shapes = json["shacl"];

        for (const auto& [key, value] : json["queries"]["triple_assembler_helper"].items()) {
            queries_dto.triple_assembler_helper[key] = value;
        }

        queries_dto.reasoning_output_queries_path = json["queries"]["output"];
        model_config_dto.queries = queries_dto;

        reasoner_settings_dto.inference_engine = json["reasoner_settings"]["inference_engine"];
        reasoner_settings_dto.is_ai_reasoner_inference_results =
            json["reasoner_settings"]["is_ai_reasoner_inference_results"];
        reasoner_settings_dto.output_format = json["reasoner_settings"]["output_format"];
        reasoner_settings_dto.supported_schema_collections =
            json["reasoner_settings"]["supported_schema_collections"];
        model_config_dto.reasoner_settings = reasoner_settings_dto;
        return model_config_dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("Failed to parse ModelConfigDTO: " + std::string(e.what()));
    }
}
