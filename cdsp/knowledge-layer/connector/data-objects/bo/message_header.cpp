#include "message_header.h"

#include "helper.h"

/**
 * @brief Constructs a MessageHeader object with the given id, and schema type.
 *
 * @param id The unique identifier for the message header. Must not be empty.
 * @param schema_type The schema type associated with the message.
 *
 * @throws std::invalid_argument if the id is empty.
 */
MessageHeader::MessageHeader(const std::string& id, const SchemaType& schema_type)
    : id_(id), schema_type_(schema_type) {
    if (id.empty()) {
        throw std::invalid_argument("MessageHeader id cannot be empty");
    }
}

/**
 * @brief Retrieves the ID of the message header.
 *
 * @return A string representing the ID of the message header.
 */
std::string MessageHeader::getId() const { return id_; }

/**
 * @brief Retrieves the schema type associated with the message header.
 *
 * @return SchemaType The schema type of the message header.
 */
SchemaType MessageHeader::getSchemaType() const { return schema_type_; }
