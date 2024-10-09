#include "message_utils.h"

/**
 * @brief Creates a JSON message header with the specified parameters.
 *
 * @param type The type of the message.
 * @param tree The tree identifier for the message.
 * @param id The unique identifier for the message.
 * @param uuid The unique identifier for the client.
 * @return A JSON object containing the message header.
 */
json createMessageHeader(const MessageType& type, const std::string& tree, const std::string& id,
                         const std::string& uuid) {
    json message_header;
    message_header["type"] = messageTypeToString(type);
    message_header["tree"] = tree;
    message_header["id"] = id;
    message_header["uuid"] = uuid;
    return message_header;
}

/**
 * @brief Creates a subscription message and adds it to the reply messages queue.
 *
 * This function generates a subscription message header using the provided
 * uuid and oid, and then appends this message to the reply messages queue.
 *
 * @param uuid Used as a client identifier.
 * @param oid The object identification number.
 * @param tree The tree identifier for the message.
 * @param reply_messages_queue A reference to the queue where the subscription message will be
 * added.
 */
void createSubscription(const std::string& uuid, const std::string& oid, const std::string& tree,
                        std::vector<json>& reply_messages_queue) {
    reply_messages_queue.push_back(createMessageHeader(MessageType::SUBSCRIBE, tree, oid, uuid));
}

/**
 * @brief Creates a read message and appends it to the reply messages queue.
 *
 * This function constructs a JSON message with a header and a list of data points,
 * then appends the message to the provided reply messages queue.
 *
 * @param uuid The client identifier.
 * @param tree The tree identifier for the message.
 * @param oid The object identification number.
 * @param data_points A vector of data point names to be included in the message.
 * @param reply_messages_queue A reference to the queue where the constructed message will be
 * appended.
 */
void createReadMessage(const std::string& uuid, const std::string& tree, const std::string& oid,
                       const std::vector<std::string>& data_points,
                       std::vector<json>& reply_messages_queue) {
    auto message = createMessageHeader(MessageType::READ, tree, oid, uuid);

    json nodes = json::array();
    for (const auto& data_point : data_points) {
        json node;
        node["name"] = data_point;
        nodes.push_back(node);
    }
    message["nodes"] = nodes;
    reply_messages_queue.push_back(std::move(message));
}

/**
 * @brief Parses a JSON message to extract error information and constructs an ErrorMessage object.
 *
 * This function handles the extraction of error details from a JSON message. The "error" field in
 * the JSON message can either be a string or an object. If it is an object, it can contain either a
 * single "node" or multiple "nodes". The function constructs the appropriate ErrorMessage object
 * based on the content of the JSON message.
 *
 * @param json_message The JSON message containing error information.
 * @return ErrorMessage The constructed ErrorMessage object containing the parsed error details.
 */
ErrorMessage parseErrorMessage(const json& json_message) {
    ErrorMessage error_message;

    error_message.type = json_message["type"].get<std::string>();
    error_message.errorCode = json_message["errorCode"].get<int>();

    // Handle the "error" field which could be either a string or an object
    if (json_message["error"].is_string()) {
        error_message.error = json_message["error"].get<std::string>();
    } else if (json_message["error"].is_object()) {
        const auto json_error = json_message["error"];
        if (json_error.contains("node")) {
            ErrorNode error_node;
            error_node.name = json_error["node"]["name"].get<std::string>();
            error_node.status = json_error["node"]["status"].get<std::string>();
            std::vector<ErrorNode> error_nodes = {error_node};
            error_message.error = error_nodes;

        } else if (json_error.contains("nodes")) {
            std::vector<ErrorNode> error_nodes;
            for (const auto& node_item : json_error["nodes"]) {
                ErrorNode error_node;
                error_node.name = node_item["name"].get<std::string>();
                error_node.status = node_item["status"].get<std::string>();
                error_nodes.push_back(error_node);
            }
            error_message.error = error_nodes;
        }
    }

    return error_message;
}

/**
 * @brief Converts a node value of a JSON object to its string representation.
 *
 * This function takes a JSON value of a node and converts it to a string based on its type.
 * It supports string, floating-point number, integer, and unsigned integer types.
 * If the JSON value is of an unsupported type, it throws a runtime error.
 *
 * @param json_value The JSON node value to be converted to a string.
 * @return A string representation of the JSON value.
 * @throws std::runtime_error If the JSON value is of an unsupported type.
 */
std::string nodeValueToString(const json& json_value) {
    if (json_value.is_string()) {
        return json_value.get<std::string>();
    } else if (json_value.is_number_float()) {
        return std::to_string(json_value.get<double>());
    } else if (json_value.is_number_integer()) {
        return std::to_string(json_value.get<int>());
    } else if (json_value.is_number_unsigned()) {
        return std::to_string(json_value.get<unsigned>());
    }
    throw std::runtime_error("The message contains a node with an unsupported value.");
}

/**
 * @brief Parses a JSON message and constructs a DataMessage object.
 *
 * This function extracts the header information and node(s) from the provided JSON message
 * and constructs a DataMessage object. The JSON message is expected to contain specific fields
 * such as "id", "type", "tree", "dateTime", and "uuid" for the header. It may also contain either
 * a single "node" or multiple "nodes".
 *
 * @param json_message The JSON message to be parsed.
 * @return DataMessage The constructed DataMessage object containing the parsed data.
 */
DataMessage parseSuccessMessage(const json& json_message) {
    MessageHeader header;

    header.id = json_message.at("id").get<std::string>();
    header.type = json_message.at("type").get<std::string>();
    header.tree = json_message.at("tree").get<std::string>();
    header.date_time = json_message.at("dateTime").get<std::string>();
    header.uuid = json_message.at("uuid").get<std::string>();

    DataMessage data_message;

    data_message.header = header;
    if (json_message.contains("node")) {
        Node node;
        node.name = json_message["node"].at("name").get<std::string>();
        node.value = nodeValueToString(json_message["node"].at("value"));
        data_message.nodes.push_back(node);
    } else if (json_message.contains("nodes")) {
        for (const auto& node_item : json_message["nodes"]) {
            Node node;
            node.name = node_item.at("name").get<std::string>();
            node.value = nodeValueToString(node_item.at("value"));
            data_message.nodes.push_back(node);
        }
    }

    return data_message;
}

/**
 * @brief Parses and displays a JSON message, returning either a DataMessage or an ErrorMessage.
 *
 * This function takes a JSON string, parses it, and prints the parsed JSON to the standard output.
 * Depending on the content of the JSON, it will return either a DataMessage or an ErrorMessage.
 *
 * @param message The JSON string to be parsed and displayed.
 * @return std::variant<DataMessage, ErrorMessage> The parsed message, which can be either a
 * DataMessage or an ErrorMessage.
 */
std::variant<DataMessage, ErrorMessage> displayAndParseMessage(const std::string& message) {
    json json_message = json::parse(message);
    std::cout << "Received message: " << json_message.dump() << std::endl << std::endl;

    if (json_message.contains("error")) {
        return parseErrorMessage(json_message);
    }

    return parseSuccessMessage(json_message);
}