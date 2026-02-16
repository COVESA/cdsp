#include "message_service.h"

#include <iostream>

#include "bo_service.h"
#include "bo_to_dto.h"
#include "dto_service.h"
#include "dto_to_bo.h"
#include "message_header.h"
#include "unsubscribe_message_dto.h"

/**
 * @brief Creates a SubscribeMessage and queues it for processing.
 *
 * This function creates a SubscribeMessage for the specified object ID and
 * schema type, including all data points in the provided data point list. It
 * then adds this message to the reply messages queue for further processing.
 *
 * @param object_id The ID of the object to subscribe to.
 * @param schema_type The schema type for which to subscribe.
 * @param data_point_list A vector of data points to subscribe to.
 * @param registry A reference to the RequestRegistry used for tracking
 * requests.
 * @param reply_messages_queue A reference to the queue where messages will be
 *                            added for processing.
 */
void MessageService::createAndQueueSubscribeMessage(const std::string &object_id,
                                                    const SchemaType &schema_type,
                                                    const std::vector<std::string> &data_point_list,
                                                    RequestRegistry &registry,
                                                    std::vector<json> &reply_messages_queue) {
    // Create a GetMessage to retrieve all data points for the schema type
    auto get_message = BoService::createGetMessage(object_id, schema_type, data_point_list);
    addMessageToQueue(get_message, registry, reply_messages_queue);

    // Start subscribing to data points into the schema type
    std::vector<Node> nodes;
    nodes.reserve(data_point_list.size());
    for (const auto &data_point : data_point_list) {
        nodes.emplace_back(data_point, std::nullopt, Metadata());
    }

    auto subscribe_message = BoService::createSubscribeMessage(object_id, schema_type, nodes);
    addMessageToQueue(subscribe_message, registry, reply_messages_queue);
}

/**
 * @brief Creates an UnsubscribeMessage and queues it for processing.
 *
 * This function creates an UnsubscribeMessage for the specified object ID,
 * schema type, and list of data points, and adds it to the reply messages queue
 * for further processing.
 *
 * @param object_id The ID of the object to unsubscribe from.
 * @param schema_type The schema type for which to unsubscribe.
 * @param data_point_list A vector of data points to unsubscribe from.
 * @param registry A reference to the RequestRegistry used for tracking
 * requests.
 * @param reply_messages_queue A reference to the queue where messages will be
 *                            added for processing.
 */
void MessageService::createAndQueueUnsubscribeMessage(
    const std::string &object_id, const SchemaType &schema_type,
    const std::vector<std::string> &data_point_list, RequestRegistry &registry,
    std::vector<json> &reply_messages_queue) {
    // Create an UnsubscribeMessage to stop subscribing to data points for the
    // schema type
    std::vector<Node> nodes;
    nodes.reserve(data_point_list.size());
    for (const auto &data_point : data_point_list) {
        nodes.emplace_back(data_point, std::nullopt, Metadata());
    }
    auto unsubscribe_message = BoService::createUnsubscribeMessage(object_id, schema_type, nodes);
    addMessageToQueue(unsubscribe_message, registry, reply_messages_queue);
}

/**
 * @brief Creates and queues SetMessage objects for setting data points.
 *
 * This function generates SetMessage objects to set data points for a given
 * schema type and queues them for processing.
 *
 * @param object_ids A map associating SchemaType with its corresponding object
 * ID. This identifies the schema type and object to be updated.
 * @param json_body A JSON object containing the data to be set in the
 * SetMessage.
 * @param registry A reference to the RequestRegistry used to track and manage
 * requests.
 * @param reply_messages_queue A reference to a vector where the generated JSON
 * messages (SetMessage and optional GetMessage) will be queued.
 * @param origin_system_name A string representing the name of the originating
 * system. This is used to identify the source of the messages.
 */
void MessageService::createAndQueueSetMessage(const std::map<SchemaType, std::string> &object_ids,
                                              const json &json_body, RequestRegistry &registry,
                                              std::vector<json> &reply_messages_queue,
                                              const std::string &origin_system_name) {
    // Create a SetMessage to set the data points for the schema type
    auto set_messages = BoService::createSetMessage(object_ids, json_body, origin_system_name);

    for (auto &set_message : set_messages) {
        const auto &schema_type = set_message.getHeader().getSchemaType();

        // Add the SetMessage to the processing queue
        addMessageToQueue(set_message, registry, reply_messages_queue);
    }
}

/**
 * @brief Processes incoming WebSocket messages and extracts DataMessage if
 * available.
 *
 * Parses JSON-RPC messages into DataMessageDTO or StatusMessageDTO. For data
 * messages, converts and returns directly. For status messages logs
 * errors/success.
 *
 * @param message JSON-RPC formatted string from WebSocket server.
 * @param registry RequestRegistry for tracking requests and DTO conversion.
 * @return DataMessage if available from data response,
 *         std::nullopt for status-only messages or errors.
 */
std::optional<DataMessage> MessageService::getDataOrProcessStatusFromMessage(
    const std::string &message, RequestRegistry &registry) {
    const auto parsed_message = MessageService::displayAndParseMessage(message);

    if (std::holds_alternative<StatusMessageDTO>(parsed_message)) {
        StatusMessageDTO status_message_dto = std::get<StatusMessageDTO>(parsed_message);
        try {
            StatusMessage status_message = DtoToBo::convert(status_message_dto, registry);

            std::cout << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                      << " Message received (Request ID:" << status_message.getIdentifier()
                      << "):\n";

            auto error = status_message.getError();
            if (error.has_value()) {
                std::cout << "  Error during processing: \n";
                std::cout << "    - Code: " << error->getCode() << "\n";
                std::cout << "    - Message: " << error->getMessage() << "\n";
                if (error->getData()) {
                    std::cout << "    - Data: " << *error->getData() << "\n";
                }
            } else {
                // Successful status message
                std::cout << " Processed successfully!\n";
            }
        } catch (const std::exception &e) {
            std::cerr << "Error parsing status message: " << e.what() << "\n";
        }
    } else {
        DataMessageDTO data_message_dto = std::get<DataMessageDTO>(parsed_message);
        try {
            return DtoToBo::convert(data_message_dto, registry);
        } catch (const std::exception &e) {
            std::cerr << "Error parsing and transforming data message to RDF triple: " << e.what()
                      << "\n";
        }
    }
    return std::nullopt;
}

/**
 * @brief Adds a message to the processing queue based on its type.
 *
 * This function takes a variant message which can be of type GetMessage,
 * SetMessage, SubscribeMessage, or UnsubscribeMessage and processes it
 * accordingly. It converts the message to its corresponding Data Transfer
 * Object (DTO) and adds it to the reply messages queue. Additionally, it
 * registers the request information in the provided RequestRegistry.
 *
 * @param message A variant containing the message to be processed. It can
 * be one of the following types: GetMessage, SetMessage, SubscribeMessage,
 *                or UnsubscribeMessage.
 * @param registry A reference to the RequestRegistry used to track requests
 *                 and assign unique IDs to the DTOs.
 * @param reply_messages_queue A reference to a vector where the converted
 *                            JSON DTOs will be pushed for further
 *                            processing.
 * @throws std::runtime_error If the message type is unknown and cannot be
 *                            processed.
 */
void MessageService::addMessageToQueue(
    const std::variant<GetMessage, SetMessage, SubscribeMessage, UnsubscribeMessage> &message,
    RequestRegistry &registry, std::vector<json> &reply_messages_queue) {
    std::visit(
        [&reply_messages_queue, &registry](const auto &specific_message) {
            auto dto = BoToDto::convert(specific_message);

            for (auto &actual_dto : dto) {
                using T = std::decay_t<decltype(actual_dto)>;
                RequestInfo request_info;

                if constexpr (std::is_same_v<T, GetMessageDTO>) {
                    request_info.type = RequestInfo::Type::GET;
                } else if constexpr (std::is_same_v<T, SubscribeMessageDTO>) {
                    request_info.type = RequestInfo::Type::SUBSCRIBE;
                } else if constexpr (std::is_same_v<T, UnsubscribeMessageDTO>) {
                    request_info.type = RequestInfo::Type::UNSUBSCRIBE;
                } else if constexpr (std::is_same_v<T, SetMessageDTO>) {
                    request_info.type = RequestInfo::Type::SET;
                } else {
                    throw std::runtime_error("Unknown DTO type in addMessageToQueue");
                }

                request_info.schema = actual_dto.schema;
                request_info.instance = actual_dto.instance;
                request_info.path = actual_dto.path;

                actual_dto.id = registry.addRequest(request_info);
                reply_messages_queue.push_back(json(actual_dto));
            }
        },
        message);
}

/**
 * @brief Parses and displays a JSON-RPC message, returning either a
 * DataMessageDTO or a StatusMessageDTO.
 *
 * This function attempts to parse the provided JSON string into a JSON object
 * and checks for the presence of a valid JSON-RPC version. Depending on the
 * content of the message, it either parses and returns a DataMessageDTO or a
 * StatusMessageDTO. If the message cannot be parsed or is invalid, an exception
 * is thrown.
 *
 * @param message The JSON string representing the message to be parsed.
 * @return A std::variant containing either a DataMessageDTO or a
 * StatusMessageDTO.
 * @throws std::invalid_argument If the JSON-RPC version is invalid.
 * @throws std::runtime_error If the message cannot be parsed or is of an
 * unknown type.
 */
std::variant<DataMessageDTO, StatusMessageDTO> MessageService::displayAndParseMessage(
    const std::string &message) {
    try {
        // Parse the input JSON string into a JSON object
        json json_message = json::parse(message);
        if (!json_message.contains("jsonrpc") || json_message["jsonrpc"] != getJsonRpcVersion()) {
            throw std::invalid_argument("Invalid JSON-RPC version");
        }

        if (json_message.contains("result") && !json_message["result"].empty()) {
            // Parse and return a DataMessage
            std::cout << "(" << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                      << ") Websocket-Server: Data message received correctly\n"
                      << "Message Content: " << json_message.dump() << "\n";
            return (DtoService::parseDataJsonToDto(json_message));
        }

        if (json_message.contains("error") ||
            (json_message.contains("result") && json_message["result"].empty())) {
            std::cout << "(" << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                      << ") Websocket-Server: Error status message received\n"
                      << "Message Content: " << json_message.dump() << "\n";
            return DtoService::parseStatusJsonToDto(json_message);
        }

        // Log and return an ErrorMessage for unknown message types
        throw std::runtime_error("The incoming message cannot be parsed: " + json_message.dump());

    } catch (const std::exception &e) {
        // Log and return an ErrorMessage for JSON parsing errors
        std::cerr << "(" << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                  << ") Websocket-Server: Error parsing JSON message: " << e.what() << "\n";
        throw std::runtime_error("Error parsing JSON message: " + std::string(e.what()));
    }
}
