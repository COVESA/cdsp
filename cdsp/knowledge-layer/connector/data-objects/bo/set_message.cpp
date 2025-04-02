#include "set_message.h"

/**
 * @brief Constructs a SetMessage object with the specified header and nodes.
 *
 * @param header The MessageHeader object that contains the header information for the message.
 * @param nodes A vector of Node objects that represent the nodes associated with the message.
 */
SetMessage::SetMessage(const MessageHeader& header, const std::vector<Node>& nodes)
    : header_(header), nodes_(nodes) {}

/**
 * @brief Retrieves the header of the SetMessage.
 *
 * This function returns the MessageHeader associated with the SetMessage instance. It provides
 * access to the header information encapsulated within the message.
 *
 * @return MessageHeader The header of the SetMessage.
 */
MessageHeader SetMessage::getHeader() const { return header_; }

/**
 * @brief Retrieves the list of nodes.
 *
 * This function returns a vector containing Node objects. It provides access to the nodes
 * associated with the SetMessage.
 *
 * @return std::vector<Node> A vector of Node objects.
 */
std::vector<Node> SetMessage::getNodes() const { return nodes_; }

/**
 * @brief Overloads the << operator to print the SetMessage object.
 *
 * This function overloads the << operator to enable printing of the SetMessage object to an output
 * stream. It allows for easy debugging and logging of the message contents.
 *
 * @param os The output stream to write to.
 * @param message The SetMessage object to print.
 * @return std::ostream& The output stream with the SetMessage object printed to it.
 */
std::ostream& operator<<(std::ostream& os, const SetMessage& message) {
    os << "SetMessage {" << "\n";
    os << "  Header: \n";
    os << "     Id: " << message.getHeader().getId() << "\n";
    os << "     Schema Type: " << schemaTypeToString(message.getHeader().getSchemaType()) << "\n";
    if (message.getNodes().empty()) {
        os << "  Nodes: None" << "\n";
    } else {
        os << "  Nodes: " << "\n";
        for (const auto& node : message.getNodes()) {
            os << "    " << node.getName() << ": " << node.getValue().value_or("") << "\n";
        }
    }
    if (message.getNodes().empty()) {
        os << "  Metadata: None" << "\n";
    } else {
        os << "  Metadata: " << "\n";
        for (const auto& node : message.getNodes()) {
            os << "    " << node.getName() << "\n";
            os << "      Received: " << node.getMetadata().getReceived().time_since_epoch().count()
               << "\n";
            if (node.getMetadata().getGenerated().has_value()) {
                os << "      Generated: "
                   << node.getMetadata().getGenerated().value().time_since_epoch().count() << "\n";
            }
        }
    }
    os << "}" << "\n";
    return os;
}