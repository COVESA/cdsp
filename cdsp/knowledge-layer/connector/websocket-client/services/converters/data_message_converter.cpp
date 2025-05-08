#include "data_message_converter.h"

#include <chrono>
#include <iostream>

#include "converter_helper.h"
#include "node.h"

/**
 * Converts a DataMessageDTO object to a DataMessage object.
 *
 * @param dto The DataMessageDTO object to be converted. It must have a type of "data".
 * @param system_data_points A map of schema types to data points used to validate the schema.
 * @return A DataMessage object constructed from the provided DataMessageDTO.
 * @throws std::invalid_argument if the dto type is not "data".
 */
DataMessage DataMessageConverter::convert(
    const DataMessageDTO& dto,
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
void DataMessageConverter::parseNodes(const std::string& base_path, const nlohmann::json& data,
                                      std::vector<Node>& nodes,
                                      const std::string& schema_collection,
                                      const std::optional<MetadataDTO>& metadata,
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
 * Extracts business object metadata from a given MetadataDTO object.
 *
 * This function attempts to retrieve and parse metadata associated with a specific path
 * from the provided MetadataDTO. If the metadata is not available or parsing fails,
 * it returns an empty optional.
 *
 * @param metadata_dto An optional MetadataDTO object containing metadata nodes.
 * @param path A string representing the path to the desired metadata node.
 * @return An optional Metadata object containing the parsed metadata if successful,
 *         or std::nullopt if the metadata is unavailable or parsing fails.
 */
Metadata DataMessageConverter::extractBoMetadata(const std::optional<MetadataDTO>& metadata_dto,
                                                 const std::string& path) {
    if (!metadata_dto.has_value() ||
        metadata_dto.value().nodes.find(path) == metadata_dto.value().nodes.end()) {
        return Metadata();
    }

    const auto& node_metadata = metadata_dto.value().nodes.at(path);
    if (node_metadata.received.seconds == 0 && node_metadata.received.nanos == 0 &&
        node_metadata.generated.seconds == 0 && node_metadata.generated.nanos == 0) {
        return Metadata();
    }

    auto received = ConverterHelper::parseTimestamp(node_metadata.received.seconds,
                                                    node_metadata.received.nanos);
    auto generated = ConverterHelper::parseTimestamp(node_metadata.generated.seconds,
                                                     node_metadata.generated.nanos);
    return Metadata(received, generated);
}
