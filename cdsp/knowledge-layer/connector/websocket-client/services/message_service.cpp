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
 * @brief Adds a message to the reply messages queue.
 *
 * This function takes a message variant and adds it to the reply messages queue. The message is
 * converted to a DTO and then to a JSON object before being added to the queue.
 *
 * @param message The message to be added to the queue.
 * @param reply_messages_queue The queue to which the message will be added.
 */
void MessageService::addMessageToQueue(
    const std::variant<GetMessage, SubscribeMessage, SetMessage>& message,
    std::vector<json>& reply_messages_queue) {
    std::visit(
        [&reply_messages_queue](const auto& specific_message) {
            auto dto = BoToDto::convert(specific_message);
            json json_dto = dto;
            if (json_dto.is_array()) {
                for (const auto& message_dto : json_dto) {
                    reply_messages_queue.push_back(message_dto);
                }
            } else {
                reply_messages_queue.push_back(json_dto);
            }
        },
        message);
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
            std::cout << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                      << " Data message received:\n"
                      << json_message.dump() << std::endl
                      << std::endl;
            return (DtoService::parseDataJsonToDto(json_message));
        } else if (type == "status") {
            // Parse and return a StatusMessage
            return DtoService::parseStatusJsonToDto(json_message);
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

/**
 * @brief Retrieves a DataMessage from a given message string or logs the status if the message is a
 * StatusMessageDTO.
 *
 * This function attempts to parse the provided message string. If the parsed message is of type
 * StatusMessageDTO, it converts it to a StatusMessage and logs its code and message. If the parsed
 * message is of type DataMessageDTO, it converts it to a DataMessage and returns it. In case of
 * any exceptions during the conversion process, an error message is logged.
 *
 * @param message A constant reference to a string containing the message to be parsed.
 * @return std::optional<DataMessage> An optional containing the converted DataMessage if
 * successful, or std::nullopt if the message is a StatusMessageDTO or an error occurs during
 * conversion.
 */
std::optional<DataMessage> MessageService::getDataMessageOrLogStatus(const std::string& message) {
    const auto parsed_message = MessageService::displayAndParseMessage(message);
    DtoToBo dto_to_bo;

    if (std::holds_alternative<StatusMessageDTO>(parsed_message)) {
        StatusMessageDTO status_message_dto = std::get<StatusMessageDTO>(parsed_message);
        try {
            StatusMessage status_message = dto_to_bo.convert(status_message_dto);

            std::cout << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
                      << " Status message received(" << status_message.getCode() << "):\n"
                      << status_message.getMessage() << std::endl
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
