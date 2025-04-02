#include "bo_service.h"

#include <iostream>

#include "helper.h"
#include "message_header.h"
#include "node.h"

/**
 * @brief Creates a SubscribeMessage object for a given object ID and schema type.
 *
 * This function initializes a MessageHeader with the provided object ID and schema type,
 * and returns a SubscribeMessage. Currently, it contains a TODO to refactor the method
 * to allow subscribing to each node in the schema collection.
 *
 * @param object_id A string representing the unique identifier for the object to subscribe to.
 * @param schema_type The SchemaType indicating the type of schema for the subscription.
 * @return A SubscribeMessage object initialized with the created MessageHeader and an empty
 * payload.
 */
SubscribeMessage BoService::createSubscribeMessage(const std::string& object_id,
                                                   SchemaType schema_type) {
    MessageHeader subscribe_header(object_id, schema_type);
    // TODO: Refactor to subscribe to each node in the schema collection
    return SubscribeMessage(subscribe_header, {});
}

/**
 * @brief Creates a GetMessage object for a given object ID and schema type.
 *
 * This function initializes a MessageHeader with the provided object ID and schema type,
 * and constructs a vector of Node objects from the specified list of data points.
 * It then returns a GetMessage object containing the header and the constructed nodes.
 *
 * @param object_id A string representing the unique identifier for the object to retrieve.
 * @param schema_type The SchemaType indicating the type of schema for the message.
 * @param list_data_points A vector of strings representing the data points to include in the
 * message.
 * @return A GetMessage object initialized with the created MessageHeader and the corresponding
 * nodes.
 */
GetMessage BoService::createGetMessage(const std::string& object_id, SchemaType schema_type,
                                       const std::vector<std::string>& list_data_points) {
    MessageHeader get_header(object_id, schema_type);
    std::vector<Node> nodes;

    for (const auto& data_point : list_data_points) {
        nodes.push_back(Node(data_point, std::nullopt, Metadata(), {data_point}));
    }

    return GetMessage(get_header, nodes);
}

/**
 * @brief Creates a vector of SetMessage objects based on the provided object ID map and JSON data.
 *
 * This function iterates through the given JSON data, extracting schema types and their
 * corresponding data points. It constructs SetMessage objects for each schema type found in the
 * JSON that matches an entry in the object ID map. If a schema type is not found in the object ID
 * map, a warning is logged.
 *
 * @param object_id A map that associates SchemaType with their corresponding object IDs.
 * @param json A JSON object containing the schema types and their associated data points.
 * @return A vector of SetMessage objects created from the provided JSON data.
 */
std::vector<SetMessage> BoService::createSetMessage(
    const std::map<SchemaType, std::string>& object_id, const nlohmann::json& json) {
    std::vector<SetMessage> set_messages;

    Metadata metadata(std::nullopt, std::chrono::system_clock::now());

    for (const auto& groups : json) {
        for (const auto& [schema, data_points] : groups.items()) {
            SchemaType schema_type = stringToSchemaType(schema);
            if (object_id.find(schema_type) == object_id.end()) {
                std::cerr << "Warning: Schema type " << schema << " not found in object ID map."
                          << std::endl;
                continue;
            }

            std::vector<Node> nodes;

            for (const auto& [data_point, value] : data_points.items()) {
                nodes.push_back(Node(data_point, value.dump(), metadata, {data_point}));
            }

            MessageHeader header(object_id.at(schema_type), schema_type);
            set_messages.push_back(SetMessage(header, nodes));
        }
    }

    return set_messages;
}