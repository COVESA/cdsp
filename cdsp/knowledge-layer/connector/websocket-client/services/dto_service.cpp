#include "dto_service.h"

#include <iostream>

#include "helper.h"
#include "metadata_dto.h"

/**
 * Parses a JSON object into a DataMessageDTO object.
 *
 * @param json The JSON object to parse, expected to contain keys: "type",
 * "schema", "instance", and optionally "path", "metadata", and "requestId".
 * @return A DataMessageDTO object populated with data extracted from the JSON
 * object.
 */
DataMessageDTO DtoService::parseDataJsonToDto(const nlohmann::json& json) {
    try {
        if (!json.contains("id")) {
            throw std::invalid_argument("Missing required id field in DataMessageDTO");
        }

        if (!json["result"].contains("data")) {
            throw std::invalid_argument("Missing required result data field in DataMessageDTO");
        }

        DataMessageDTO dto;
        dto.id = json["id"].get<int>();
        dto.data = json["result"]["data"].get<nlohmann::json>();
        dto.metadata =
            json["result"].contains("metadata")
                ? std::optional<MetadataDTO>(parseMetadataJsonToDto(json["result"]["metadata"]))
                : std::nullopt;
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
        StatusMessageDTO dto;
        dto.id = json["id"];
        if (json.contains("error")) {
            StatusMessageErrorDTO error;

            if (json.contains("error") && json["error"].contains("code")) {
                error.code = json["error"]["code"].get<int>();
            }

            if (json["error"].contains("message")) {
                error.message = json["error"]["message"].get<std::string>();
            }

            if (json["error"].contains("data")) {
                error.data = json["error"]["data"].get<nlohmann::json>();
            } else {
                error.data = "";
            }
            dto.error = error;
        } else if (json.contains("result")) {
            dto.error = std::nullopt;
        } else {
            throw std::invalid_argument(
                "Invalid StatusMessageDTO: missing error or "
                "result field");
        }

        return dto;
    } catch (const std::exception& e) {
        std::cout << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                  << " Status message received:\n"
                  << json.dump() << std::endl
                  << std::endl;
        std::cerr << " Failed to parse StatusMessageDTO: " << e.what() << std::endl;
        return {};
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
        // Helper lambda to parse timestamp data
        auto parseTimestamp = [](const nlohmann::json& timestamp_json,
                                 const std::string& timestamp_type) -> MetadataDTO::Timestamp {
            if (!timestamp_json.contains("seconds")) {
                throw std::invalid_argument("Missing required seconds field into " +
                                            timestamp_type + " timestamp in MetadataDTO");
            }

            MetadataDTO::Timestamp timestamp;
            timestamp.seconds = timestamp_json["seconds"].get<int64_t>();

            if (timestamp_json.contains("nanos")) {
                timestamp.nanos = timestamp_json["nanos"].get<int64_t>();
            }

            return timestamp;
        };

        // Helper lambda to parse confidence data
        auto parseConfidence =
            [](const nlohmann::json& confidence_json) -> MetadataDTO::Confidence {
            if (!confidence_json.contains("type") || !confidence_json.contains("value")) {
                throw std::invalid_argument("Missing required confidence fields in MetadataDTO");
            }

            return MetadataDTO::Confidence{confidence_json["type"].get<std::string>(),
                                           confidence_json["value"].get<int>()};
        };

        for (const auto& [node, metadata] : metadata_json.items()) {
            MetadataDTO::NodeMetadata node_metadata;

            // Parse received timestamps
            if (metadata.contains("timestamps") && metadata["timestamps"].contains("received")) {
                node_metadata.received =
                    parseTimestamp(metadata["timestamps"]["received"], "received");
            }

            // Parse generated timestamps
            if (metadata.contains("timestamps") && metadata["timestamps"].contains("generated")) {
                node_metadata.generated =
                    parseTimestamp(metadata["timestamps"]["generated"], "generated");
            }

            // Parse confidence
            if (metadata.contains("confidence")) {
                node_metadata.confidence = parseConfidence(metadata["confidence"]);
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
        if (!json.contains("inputs")) {
            throw std::invalid_argument("Missing required inputs field in ModelConfigDTO");
        }

        if (!json.contains("ontologies")) {
            throw std::invalid_argument("Missing required ontologies field in ModelConfigDTO");
        }

        if (!json.contains("output")) {
            throw std::invalid_argument("Missing required output field in ModelConfigDTO");
        }

        if (!json.contains("rules")) {
            throw std::invalid_argument("Missing required rules field in ModelConfigDTO");
        }

        if (!json.contains("shacl")) {
            throw std::invalid_argument("Missing required shacl field in ModelConfigDTO");
        }

        if (!json.contains("queries")) {
            throw std::invalid_argument("Missing required queries field in ModelConfigDTO");
        }

        if (!json.contains("reasoner_settings")) {
            throw std::invalid_argument(
                "Missing required reasoner_settings field in ModelConfigDTO");
        }

        ModelConfigDTO model_config_dto;

        model_config_dto.inputs = json["inputs"];
        model_config_dto.ontologies = json["ontologies"];
        model_config_dto.output = json["output"];
        model_config_dto.rules = json["rules"];
        model_config_dto.shacl_shapes = json["shacl"];

        QueriesDTO queries_dto;
        for (const auto& [key, value] : json["queries"]["triple_assembler_helper"].items()) {
            queries_dto.triple_assembler_helper[key] = value;
        }

        queries_dto.reasoning_output_queries_path = json["queries"]["output"];
        model_config_dto.queries = queries_dto;

        ReasonerSettingsDTO reasoner_settings_dto;
        reasoner_settings_dto = parseReasonerSettingsJsonToDto(json["reasoner_settings"]);
        model_config_dto.reasoner_settings = reasoner_settings_dto;

        return model_config_dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("Failed to parse ModelConfigDTO: " + std::string(e.what()));
    }
}

/**
 * Parses a JSON object containing reasoner settings into a ReasonerSettingsDTO.
 *
 * @param reasoner_settings_json The JSON object containing the reasoner
 * settings data.
 * @return A ReasonerSettingsDTO object populated with data from the JSON
 * object.
 */
ReasonerSettingsDTO DtoService::parseReasonerSettingsJsonToDto(
    const nlohmann::json& reasoner_settings_json) {
    ReasonerSettingsDTO dto;
    try {
        if (!reasoner_settings_json.contains("inference_engine")) {
            throw std::invalid_argument(
                "Missing required inference_engine field in ReasonerSettingsDTO");
        }

        if (!reasoner_settings_json.contains("output_format")) {
            throw std::invalid_argument(
                "Missing required output_format field in ReasonerSettingsDTO");
        }

        if (!reasoner_settings_json.contains("supported_schema_collections")) {
            throw std::invalid_argument(
                "Missing required supported_schema_collections field in "
                "ReasonerSettingsDTO");
        }

        dto.inference_engine = reasoner_settings_json["inference_engine"];
        dto.output_format = reasoner_settings_json["output_format"];
        dto.supported_schema_collections =
            reasoner_settings_json["supported_schema_collections"].get<std::vector<std::string>>();

        if (reasoner_settings_json.contains("is_ai_reasoner_inference_results")) {
            dto.is_ai_reasoner_inference_results =
                reasoner_settings_json["is_ai_reasoner_inference_results"];
        }

        return dto;
    } catch (const nlohmann::json::exception& e) {
        throw std::invalid_argument("ReasonerSettingsDTO: " + std::string(e.what()));
    }
}
