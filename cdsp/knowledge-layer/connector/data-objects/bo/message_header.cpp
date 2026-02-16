#include "message_header.h"

/**
 * @brief Constructs a MessageHeader object with the given instance, and schema
 * type.
 *
 * @param instance The instance for the message header. Must not be empty.
 * @param schema_type The schema type associated with the message.
 *
 * @throws std::invalid_argument if the instance is empty.
 */
MessageHeader::MessageHeader(const std::string& instance, const SchemaType& schema_type)
    : instance_(instance), schema_type_(schema_type) {
    if (instance.empty()) {
        throw std::invalid_argument("MessageHeader instance cannot be empty");
    }
}

/**
 * @brief Retrieves the instance of the message header.
 *
 * @return A string representing the instance of the message header.
 */
std::string MessageHeader::getInstance() const { return instance_; }
/**
 * @brief Retrieves the schema type associated with the message header.
 *
 * @return SchemaType The schema type of the message header.
 */
SchemaType MessageHeader::getSchemaType() const { return schema_type_; }
