#include "data_message_converter.h"

#include <iostream>

#include "converter_helper.h"
#include "node.h"

/**
 * @brief DataMessageConverter is responsible for converting DataMessageDTO
 * objects, which are typically received as JSON payloads, into DataMessage
 * objects that encapsulate structured data nodes and their associated metadata.
 *
 * Converts a DataMessageDTO object to a DataMessage object.
 *
 * @param dto The DataMessageDTO object to be converted. It must have a type of
 * "data".
 * @param registry The RequestRegistry object that contains information about
 * the request associated with the DTO.
 * @return A DataMessage object constructed from the provided DataMessageDTO.
 * @throws std::invalid_argument if the dto type is not "data".
 */
DataMessage DataMessageConverter::convert(const DataMessageDTO &dto, RequestRegistry &registry) {
    // Check if the message belongs to the request registry
    auto request_registry = registry.getRequest(dto.id);

    if (!request_registry.has_value()) {
        throw std::invalid_argument("Registry not found for the ID: " + std::to_string(dto.id));
    }

    SchemaType schema_type = stringToSchemaType(request_registry->schema);

    MessageHeader header(request_registry->instance, schema_type);

    std::vector<Node> nodes;
    std::string base_path = request_registry->path.value_or("");

    if (base_path.empty() && !dto.data.is_object()) {
        throw std::invalid_argument("Path is missing and data is not an object");
    }

    parseNodes(base_path, dto.data, nodes, request_registry->schema);

    // Combine nodes with their associated metadata to produce the final node
    // collection
    std::vector<Node> nodes_with_metadata =
        includeMetadata(request_registry->schema, base_path, dto.metadata, nodes);

    if (request_registry->type != RequestInfo::Type::SUBSCRIBE) {
        // Remove the request from the registry after processing
        registry.removeRequest(dto.id);
    }

    return {header, nodes_with_metadata};
}

/**
 * @brief Parses a JSON object and converts it into a vector of Node objects.
 *
 * This function recursively traverses a JSON object, constructing a path for
 * each key-value pair. If a value is an object, the function calls itself
 * recursively. Otherwise, it creates a Node object with the constructed path,
 * the value converted to a string, and optional metadata, then adds it to the
 * nodes vector.
 *
 * @param base_path The base path used to construct the full path for each node.
 * @param data The JSON object to parse.
 * @param nodes A reference to a vector where the resulting Node objects will be
 * stored.
 * @param schema_collection A string representing the schema used to prefix the
 * node names.
 * @param metadata Optional metadata that can be associated with each node.
 */
void DataMessageConverter::parseNodes(const std::string &base_path, const nlohmann::json &data,
                                      std::vector<Node> &nodes,
                                      const std::string &schema_collection) {
    for (const auto &[key, value] : data.items()) {
        std::string path;
        if (key.empty()) {
            // Leaf message
            path = base_path;
        } else if (base_path.empty()) {
            path = key;
        } else {
            // Detect array index by checking if key is numeric (used to correctly
            // format paths)
            bool is_numeric = std::all_of(key.begin(), key.end(), ::isdigit);
            path = base_path;
            if (is_numeric) {
                path.append("[").append(key).append("]");
            } else {
                path.append(".").append(key);
            }
        }

        if (value.is_object()) {
            parseNodes(path, value, nodes, schema_collection);
        } else if (value.is_array()) {
            for (size_t i = 0; i < value.size(); ++i) {
                std::string indexed_path = path + "[" + std::to_string(i) + "]";
                const auto &element = value[i];

                if (element.is_primitive()) {
                    // Handle primitive values in the array
                    std::string name = schema_collection;
                    name.append(".").append(indexed_path);
                    auto node_value = Helper::jsonToString(element);
                    nodes.emplace_back(name, node_value, Metadata());
                } else {
                    // Recursively parse nested objects in the array
                    parseNodes(indexed_path, value[i], nodes, schema_collection);
                }
            }
        } else {
            try {
                std::string name = schema_collection;
                name.append(".").append(path);
                auto node_value = Helper::jsonToString(value);
                nodes.emplace_back(name, node_value, Metadata());
            } catch (const std::invalid_argument &e) {
                std::cerr << "Failed to create node: " << e.what() << std::endl;
            }
        }
    }
}

/**
 * @brief Includes metadata for a collection of nodes based on the provided
 * schema and metadata DTO.
 *
 * This function processes a vector of nodes and associates them with metadata
 * if available. If no metadata is provided, it creates nodes with default
 * metadata. For each node, it determines the appropriate metadata path and
 * retrieves the corresponding metadata, if it exists.
 *
 * @param schema_collection The schema collection name used to construct the
 * metadata path.
 * @param base_path The base path used in conjunction with the schema collection
 * to identify nodes.
 * @param metadata_dto An optional MetadataDTO containing metadata information
 * for the nodes.
 * @param nodes A vector of Node objects to which metadata will be added.
 * @return A vector of Node objects that includes metadata.
 */
std::vector<Node> DataMessageConverter::includeMetadata(
    const std::string &schema_collection, const std::string &base_path,
    const std::optional<MetadataDTO> &metadata_dto, const std::vector<Node> &nodes) {
    std::vector<Node> nodes_with_metadata;

    for (const auto &node : nodes) {
        if (!metadata_dto.has_value()) {
            nodes_with_metadata.emplace_back(node.getName(), node.getValue(), Metadata());
        } else {
            std::string metadata_path =
                normalizeMetadataPath(schema_collection, {base_path, node.getName()});
            const auto &metadata = findMetadata(metadata_dto->nodes, metadata_path);
            nodes_with_metadata.emplace_back(node.getName(), node.getValue(), metadata);
        }
    }

    return nodes_with_metadata;
}
/**
 * @brief Normalizes the metadata path by removing the schema collection prefix
 * and any leading dots.
 *
 * This function takes a schema collection name and a MetadataPathComponents
 * object, constructs the metadata path, and removes the schema collection
 * prefix. It also handles leading dots and array indices in the path.
 *
 * @param schema_collection The schema collection name used to construct the
 * metadata path.
 * @param components The MetadataPathComponents object containing base path and
 * node name.
 * @return A normalized metadata path as a string.
 */

std::string DataMessageConverter::normalizeMetadataPath(const std::string &schema_collection,
                                                        const MetadataPathComponents &components) {
    std::string prefix_to_remove = schema_collection + "." + components.base_path;
    std::string metadata_path = (components.node_name != prefix_to_remove)
                                    ? components.node_name.substr(prefix_to_remove.size())
                                    : "";

    if (!metadata_path.empty() && metadata_path[0] == '.') {
        metadata_path = metadata_path.substr(1);
    }

    auto pos = metadata_path.find('[');
    if (pos != std::string::npos) {
        metadata_path = metadata_path.substr(0, pos);
    }

    return metadata_path;
}

/**
 * @brief Finds the metadata for a given metadata path in the provided nodes
 * map.
 *
 * This function searches for the specified metadata path in the nodes map. If
 * not found, it defaults to an empty string. It retrieves the received and
 * generated timestamps, as well as confidence information if available.
 *
 * @param nodes A map of metadata paths to NodeMetadata objects.
 * @param metadata_path The metadata path to search for.
 * @return A Metadata object containing the received and generated timestamps,
 * and confidence information if available.
 */
Metadata DataMessageConverter::findMetadata(
    const std::unordered_map<std::string, MetadataDTO::NodeMetadata> &nodes,
    const std::string &metadata_path) {
    auto iterator = nodes.find(metadata_path);
    if (iterator == nodes.end()) {
        iterator = nodes.find("");
        if (iterator == nodes.end()) {
            return Metadata();
        }
    }

    const auto &node_metadata = iterator->second;
    auto received = (node_metadata.received.seconds != 0 || node_metadata.received.nanos != 0)
                        ? ConverterHelper::parseTimestamp(node_metadata.received.seconds,
                                                          node_metadata.received.nanos)
                        : std::nullopt;

    auto generated = (node_metadata.generated.seconds != 0 || node_metadata.generated.nanos != 0)
                         ? ConverterHelper::parseTimestamp(node_metadata.generated.seconds,
                                                           node_metadata.generated.nanos)
                         : std::nullopt;

    std::optional<std::pair<ConfidenceType, std::string>> confidence = std::nullopt;
    if (node_metadata.confidence.has_value()) {
        confidence = std::make_pair(stringToConfidenceType(node_metadata.confidence->type),
                                    std::to_string(node_metadata.confidence->value));
    }

    std::optional<Metadata::OriginType> origin_type = std::nullopt;
    if (node_metadata.origin_type.has_value()) {
        origin_type = Metadata::OriginType{.name = node_metadata.origin_type->name,
                                           .uri = node_metadata.origin_type->uri};
    }

    Metadata::Timestamps timestamps{received, generated};

    return Metadata(timestamps, origin_type, confidence);
}