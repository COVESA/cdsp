#include "subscribe_message.h"

/**
 * @brief Constructs a SubscribeMessage object with the given header and nodes.
 *
 * @param header The message header to initialize the SubscribeMessage with.
 * @param nodes A vector of Node objects to initialize the SubscribeMessage with.
 *
 * @throws std::invalid_argument if the nodes vector is empty for subscribe messages.
 */
SubscribeMessage::SubscribeMessage(const MessageHeader& header, const std::vector<Node>& nodes)
    : header_(header), nodes_(nodes) {}
/**
 * @brief Retrieves the header of the SubscribeMessage.
 *
 * This function returns the MessageHeader associated with the
 * SubscribeMessage instance. It provides access to the header
 * information encapsulated within the message.
 *
 * @return MessageHeader The header of the SubscribeMessage.
 */
MessageHeader SubscribeMessage::getHeader() const { return header_; }

/**
 * @brief Retrieves the list of nodes.
 *
 * This function returns a vector containing Node objects.
 * It provides access to the nodes associated with the SubscribeMessage.
 *
 * @return std::vector<Node> A vector of Node objects.
 */
std::vector<Node> SubscribeMessage::getNodes() const { return nodes_; }

/**
 * @brief Overloads the << operator to print the SubscribeMessage object.
 *
 * This function overloads the << operator to enable printing of
 * the SubscribeMessage object to an output stream. It allows for easy
 * debugging and logging of the message contents.
 *
 * @param os The output stream to write to.
 * @param message The SubscribeMessage object to print.
 * @return std::ostream& The output stream with the SubscribeMessage object printed to it.
 */
std::ostream& operator<<(std::ostream& os, const SubscribeMessage& message) {
    os << "SubscribeMessage {" << "\n";
    os << "  Header: \n";
    os << "     Id: " << message.getHeader().getId() << "\n";
    os << "     Schema Type: " << schemaTypeToString(message.getHeader().getSchemaType()) << "\n";
    if (message.getNodes().empty()) {
        os << "  Nodes: None" << "\n";
    } else {
        os << "  Nodes: " << "\n";
        for (const auto& node : message.getNodes()) {
            os << "    " << node.getName() << "\n";
        }
    }
    os << "}" << "\n";
    return os;
}