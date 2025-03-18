#include "node.h"

#include <algorithm>
#include <stdexcept>
/**
 * @brief Constructs a new Node object.
 *
 * @param name The name of the node. Must not be empty.
 * @param value The optional value associated with the node.
 * @param metadata The metadata associated with the node.
 * @param supported_data_points A vector of supported data points.
 *
 * @throws std::invalid_argument if the name is empty.
 * @throws std::invalid_argument if the name is not supported.
 */
Node::Node(const std::string& name, const std::optional<std::string>& value,
           const Metadata& metadata, const std::vector<std::string>& supported_data_points)
    : name_(name), value_(value), metadata_(metadata) {
    if (name.empty()) {
        throw std::invalid_argument("Node name cannot be empty");
    }
    if (supported_data_points.empty() ||
        std::find(supported_data_points.begin(), supported_data_points.end(), name) ==
            supported_data_points.end()) {
        throw std::invalid_argument("Node name `" + name + "` is not supported");
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
 * @return std::optional<std::string> An optional containing the value if it exists,
 */
std::optional<std::string> Node::getValue() const { return value_; }

/**
 * @brief Retrieves the metadata associated with the node.
 *
 * @return Metadata The metadata object associated with the node.
 */
Metadata Node::getMetadata() const { return metadata_; }
