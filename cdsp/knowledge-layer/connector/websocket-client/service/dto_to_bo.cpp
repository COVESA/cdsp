#include "dto_to_bo.h"

#include <chrono>
#include <iostream>

#include "node.h"

/**
 * Converts a DataMessageDto object to a DataMessage object.
 *
 * @param dto The DataMessageDto object to be converted. It must have a type of "data".
 * @param system_data_points A map of schema types to data points used to validate the schema.
 * @return A DataMessage object constructed from the provided DataMessageDto.
 * @throws std::invalid_argument if the dto type is not "data".
 */
DataMessage DtoToBo::convert(
    const DataMessageDto& dto,
    const std::map<SchemaType, std::vector<std::string>>& system_data_points) {
    if (dto.type != "data") {
        throw std::invalid_argument("Invalid DTO type");
    }
    if ((!dto.path.has_value() || dto.path->empty()) && !dto.data.is_object()) {
        throw std::invalid_argument("Path is missing");
    }
    SchemaType schema_type = stringToSchemaType(dto.schema);
    if (system_data_points.find(schema_type) == system_data_points.end()) {
        throw std::invalid_argument("Schema not found in system data points");
    }
    const std::vector<std::string>& supported_data_points = system_data_points.at(schema_type);

    MessageHeader header(dto.instance, schema_type);

    std::vector<Node> nodes;
    std::string base_path = dto.path.value_or("");
    parseNodes(base_path, dto.data, nodes, dto.schema, dto.metadata, supported_data_points);

    return DataMessage(header, nodes);
}

/**
 * Converts a StatusMessageDto object to a StatusMessage object.
 *
 * @param dto The StatusMessageDto object containing the data to be converted.
 * @return A StatusMessage object initialized with the data from the dto.
 */
StatusMessage DtoToBo::convert(const StatusMessageDto& dto) {
    auto timestamp = parseTimestamp(dto.timestamp.seconds, dto.timestamp.nanoseconds);
    if (!timestamp.has_value()) {
        throw std::invalid_argument("Invalid timestamp");
    }
    return StatusMessage(dto.code, dto.message, dto.requestId, timestamp.value());
}

/**
 * @brief Parses a JSON object and converts it into a vector of Node objects.
 *
 * This function recursively traverses a JSON object, constructing a path for each
 * key-value pair. If a value is an object, the function calls itself recursively.
 * Otherwise, it creates a Node object with the constructed path, the value converted
 * to a string, and optional metadata, then adds it to the nodes vector.
 *
 * @param base_path The base path used to construct the full path for each node.
 * @param data The JSON object to parse.
 * @param nodes A reference to a vector where the resulting Node objects will be stored.
 * @param schema_collection A string representing the schema used to prefix the node names.
 * @param metadata Optional metadata that can be associated with each node.
 * @param supported_data_points A vector of supported data points for the schema.
 */
void DtoToBo::parseNodes(const std::string& base_path, const nlohmann::json& data,
                         std::vector<Node>& nodes, const std::string& schema_collection,
                         const std::optional<MetadataDto>& metadata,
                         const std::vector<std::string>& supported_data_points) {
    for (const auto& [key, value] : data.items()) {
        std::string path;
        if (key.empty()) {
            // leaf message
            path = base_path;
        } else {
            // nested or flat messages
            path = base_path.empty() ? key : base_path + "." + key;
        }

        if (value.is_object()) {
            parseNodes(path, value, nodes, schema_collection, metadata, supported_data_points);
        } else {
            try {
                std::string name = schema_collection + "." + path;
                auto node_value = Helper::jsonToString(value);
                Metadata node_metadata = extractBoMetadata(metadata, path);
                nodes.emplace_back(name, node_value, node_metadata, supported_data_points);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Failed to create node: " << e.what() << std::endl;
            }
        }
    }
}

/**
 * Extracts business object metadata from a given MetadataDto object.
 *
 * This function attempts to retrieve and parse metadata associated with a specific path
 * from the provided MetadataDto. If the metadata is not available or parsing fails,
 * it returns an empty optional.
 *
 * @param metadata_dto An optional MetadataDto object containing metadata nodes.
 * @param path A string representing the path to the desired metadata node.
 * @return An optional Metadata object containing the parsed metadata if successful,
 *         or std::nullopt if the metadata is unavailable or parsing fails.
 */
Metadata DtoToBo::extractBoMetadata(const std::optional<MetadataDto>& metadata_dto,
                                    const std::string& path) {
    if (!metadata_dto.has_value() ||
        metadata_dto.value().nodes.find(path) == metadata_dto.value().nodes.end()) {
        return Metadata();
    }

    const auto& node_metadata = metadata_dto.value().nodes.at(path);
    if (node_metadata.received.seconds == 0 && node_metadata.received.nanoseconds == 0 &&
        node_metadata.generated.seconds == 0 && node_metadata.generated.nanoseconds == 0) {
        return Metadata();
    }

    auto received =
        parseTimestamp(node_metadata.received.seconds, node_metadata.received.nanoseconds);
    auto generated =
        parseTimestamp(node_metadata.generated.seconds, node_metadata.generated.nanoseconds);
    return Metadata(received, generated);
}

/**
 * Parses the given seconds and nanoseconds into a system clock time point.
 *
 * This function attempts to create a std::chrono::system_clock::time_point from the provided
 * seconds and nanoseconds. If an exception is thrown during the conversion, it catches the
 * exception, logs an error message, and returns std::nullopt.
 *
 * @param seconds The number of seconds since the epoch.
 * @param nanos The number of nanoseconds to add to the seconds.
 * @return An optional containing the parsed time point if successful, or std::nullopt if an error
 * occurs.
 */
std::optional<std::chrono::system_clock::time_point> DtoToBo::parseTimestamp(int64_t seconds,
                                                                             int64_t nanos) {
    try {
        if (seconds <= 0 || nanos < 0 || nanos >= 1'000'000'000) {
            return std::nullopt;
        }

        return Helper::convertToTimestamp(seconds, nanos);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse timestamp: " << e.what() << std::endl;
        return std::nullopt;
    }
}
