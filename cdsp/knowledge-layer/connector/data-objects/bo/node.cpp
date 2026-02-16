#include "node.h"

#include <stdexcept>
/**
 * @brief Constructs a new Node object.
 *
 * @param name The name of the node. Must not be empty.
 * @param value The optional value associated with the node.
 * @param metadata The metadata associated with the node.
 *
 * @throws std::invalid_argument if the name is empty.
 */
Node::Node(std::string name, std::optional<std::string> value, Metadata metadata)
    : name_(std::move(name)), value_(std::move(value)), metadata_(std::move(metadata)) {
    if (name_.empty()) {
        throw std::invalid_argument("Node name cannot be empty");
    }
}

/**
 * @brief Retrieves the name of the node.
 *
 * @return A string representing the name of the node.
 */
std::string Node::getName() const { return name_; }

/**
 * @brief Retrieves the value stored in the node.
 *
 * @return std::optional<std::string> An optional containing the value if it
 * exists,
 */
std::optional<std::string> Node::getValue() const { return value_; }

/**
 * @brief Retrieves the metadata associated with the node.
 *
 * @return Metadata The metadata object associated with the node.
 */
Metadata Node::getMetadata() const { return metadata_; }
