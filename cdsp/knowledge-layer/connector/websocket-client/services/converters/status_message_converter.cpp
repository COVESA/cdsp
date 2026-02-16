#include "status_message_converter.h"

#include <iostream>

#include "error.h"
/**
 * Converts a StatusMessageDTO to a StatusMessage.
 *
 * This function checks if the provided StatusMessageDTO belongs to the
 * given RequestRegistry. If the request is found, it processes the error
 * information if available and constructs a StatusMessage object.
 *
 * @param dto The StatusMessageDTO object containing the status message data.
 * @param registry The RequestRegistry that holds the request information.
 * @return A StatusMessage object constructed from the provided DTO.
 * @throws std::invalid_argument If the request ID does not exist in the
 * registry.
 */
StatusMessage StatusMessageConverter::convert(const StatusMessageDTO &dto,
                                              RequestRegistry &registry) {
    // Check if the message belongs to the request registry
    auto request_registry = registry.getRequest(dto.id);

    if (!request_registry.has_value()) {
        throw std::invalid_argument("Registry not found for the ID: " + std::to_string(dto.id));
    }

    std::optional<Error> error;

    if (dto.error.has_value()) {
        std::optional<nlohmann::json> data = std::nullopt;
        if (!dto.error->data.empty()) {
            data = dto.error->data;
        }
        error = Error(dto.error->code, dto.error->message, data);
    }

    if (request_registry->type != RequestInfo::Type::SUBSCRIBE) {
        if (request_registry->type == RequestInfo::Type::UNSUBSCRIBE) {
            auto subscribe_identifier = registry.findRequestId(
                RequestInfo{RequestInfo::Type::SUBSCRIBE, request_registry->schema,
                            request_registry->instance, request_registry->path});
            if (subscribe_identifier.has_value()) {
                registry.removeRequest(*subscribe_identifier);
            } else {
                std::cout
                    << "Info: No matching subscribe request found for unsubscribe message ID: "
                    << dto.id << std::endl;
            }
        }
        // Remove the request from the registry after processing
        registry.removeRequest(dto.id);
    }

    return {dto.id, error};
}
