#include "status_message_converter.h"

#include "converter_helper.h"

/**
 * Converts a StatusMessageDTO object to a StatusMessage object.
 *
 * @param dto The StatusMessageDTO object containing the data to be converted.
 * @return A StatusMessage object initialized with the data from the dto.
 */
StatusMessage StatusMessageConverter::convert(const StatusMessageDTO& dto) {
    auto timestamp = ConverterHelper::parseTimestamp(dto.timestamp.seconds, dto.timestamp.nanos);
    if (!timestamp.has_value()) {
        throw std::invalid_argument("Invalid timestamp");
    }
    return StatusMessage(dto.code, dto.message, dto.requestId, timestamp.value());
}