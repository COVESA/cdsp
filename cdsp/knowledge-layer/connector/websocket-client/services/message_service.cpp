#include "message_service.h"

#include <iostream>

#include "bo_to_dto.h"
#include "dto_service.h"
#include "dto_to_bo.h"
#include "message_header.h"

MessageService::MessageService(
    const std::map<SchemaType, std::vector<std::string>>& system_data_point)
    : system_data_point_(system_data_point) {
    if (system_data_point_.empty()) {
        throw std::invalid_argument("System data points cannot be empty");
    }
}

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
void MessageService::addMessageToQueue(const GetMessage& message,
                                       std::vector<json>& reply_messages_queue) {
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
void MessageService::addMessageToQueue(const SubscribeMessage& message,
                                       std::vector<json>& reply_messages_queue) {
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
std::variant<DataMessageDTO, StatusMessageDTO> MessageService::displayAndParseMessage(
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
            throw std::runtime_error("Unknown message type: " + type);
        }
    } catch (const std::exception& e) {
        // Log and return an ErrorMessage for JSON parsing errors
        std::cerr << "Error parsing JSON message: " << e.what() << std::endl;
        throw std::runtime_error("Error parsing JSON message: " + std::string(e.what()));
    }
}

std::optional<DataMessage> MessageService::getDataMessageOrLogStatus(const std::string& message) {
    const auto parsed_message = MessageService::displayAndParseMessage(message);
    DtoToBo dto_to_bo;

    if (std::holds_alternative<StatusMessageDTO>(parsed_message)) {
        StatusMessageDTO status_message_dto = std::get<StatusMessageDTO>(parsed_message);
        try {
            StatusMessage status_message = dto_to_bo.convert(status_message_dto);
            std::cout << "Status message received(" << status_message.getCode()
                      << "): " << status_message.getMessage() << std::endl
                      << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing status message: " << e.what() << std::endl;
        }
    } else {
        DataMessageDTO data_message_dto = std::get<DataMessageDTO>(parsed_message);
        try {
            return dto_to_bo.convert(data_message_dto, system_data_point_);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing and transforming data message to RDF triple: " << e.what()
                      << std::endl;
        }
    }
    return std::nullopt;
}
