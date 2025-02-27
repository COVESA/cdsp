#include "message_utils.h"

#include <iostream>

#include "bo_to_dto.h"
#include "dto_service.h"
#include "message_header.h"

namespace MessageUtils {

/**
 * @brief Adds a `get` message to the reply messages queue.
 *
 * This function converts a given message into a DTO (Data Transfer Object) and
 * then iterates over the DTO to convert each part into a JSON object. Each JSON
 * object is subsequently added to the reply messages queue.
 *
 * @param message The message to be converted and added to the queue.
 * @param reply_messages_queue The queue where the converted JSON messages will be stored.
 */
void addMessageToQueue(const GetMessage& message, std::vector<json>& reply_messages_queue) {
    auto dto = BoToDto::convert(message, std::nullopt);
    for (const auto& message_dto : dto) {
        json json_message = message_dto;
        reply_messages_queue.push_back(json_message);
    }
}

/**
 * @brief Adds a `subscribe` message to the reply messages queue.
 *
 * This function converts a SubscribeMessage object into a DTO (Data Transfer Object)
 * and then serializes each DTO into a JSON object. The resulting JSON objects are
 * added to the provided reply messages queue.
 *
 * @param message The SubscribeMessage object to be converted and added to the queue.
 * @param reply_messages_queue A reference to the vector of JSON objects where the
 *        converted messages will be stored.
 */
void addMessageToQueue(const SubscribeMessage& message, std::vector<json>& reply_messages_queue) {
    auto dto = BoToDto::convert(message, std::nullopt);
    for (const auto& message_dto : dto) {
        json json_message = message_dto;
        reply_messages_queue.push_back(json_message);
    }
}

/**
 * @brief Parses a JSON message and returns a variant containing either a DTODataMessage,
 * DTOStatusMessage, or InternalErrorMessage.
 *
 * This function attempts to parse the input JSON string and determine the type of message it
 * contains. It supports "data" and "status" message types. If the message type is unknown or if
 * there is a parsing error, an InternalErrorMessage is returned.
 *
 * @param message A JSON string representing the message to be parsed.
 * @return A std::variant containing either a DTODataMessage, DTOStatusMessage, or ErrorMessage.
 */
std::variant<DataMessageDto, StatusMessageDto, InternalErrorMessage> displayAndParseMessage(
    const std::string& message) {
    try {
        // Parse the input JSON string into a JSON object
        json json_message = json::parse(message);

        // Extract "type" field to determine the message type
        std::string type = json_message["type"];

        if (type == "data") {
            // Parse and return a DataMessage
            std::cout << "Data message received: " << json_message.dump() << std::endl << std::endl;
            return (DtoService::parseDataDto(json_message));
        } else if (type == "status") {
            // Parse and return a StatusMessage
            return DtoService::parseStatusDto(json_message);
        } else {
            // Log and return an ErrorMessage for unknown message types
            std::cerr << "Unknown message type: " << type << std::endl;
            return InternalErrorMessage{"InvalidMessage", 400, "Unknown message type"};
        }
    } catch (const std::exception& e) {
        // Log and return an ErrorMessage for JSON parsing errors
        std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        return InternalErrorMessage{"ParseError", 500, e.what()};
    }
}
}  // namespace MessageUtils