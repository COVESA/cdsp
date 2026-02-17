#include "unsubscribe_message.h"

UnsubscribeMessage::UnsubscribeMessage(MessageHeader header, std::vector<Node> nodes)
    : header_(std::move(header)), nodes_(std::move(nodes)) {}

/**
 * @brief Retrieves the message header associated with the unsubscribe message.
 *
 * This function returns the header of the unsubscribe message, which contains
 * metadata and information relevant to the message. The header is returned as
 * a MessageHeader object.
 *
 * @return MessageHeader The header of the unsubscribe message.
 */
MessageHeader UnsubscribeMessage::getHeader() const { return header_; }

/**
 * @brief Retrieves the list of nodes associated with the UnsubscribeMessage.
 *
 * @return A vector containing Node objects.
 */
std::vector<Node> UnsubscribeMessage::getNodes() const { return nodes_; }

/**
 * Overloads the insertion (<<) operator for the UnsubscribeMessage class.
 *
 * This function formats and outputs the contents of an UnsubscribeMessage
 * object to the provided output stream. It includes the header information
 * and the list of nodes associated with the message.
 *
 * @param out_stream The output stream to which the UnsubscribeMessage will be
 * written.
 * @param message The UnsubscribeMessage object to be output.
 * @return A reference to the output stream after the UnsubscribeMessage has
 * been written.
 */
std::ostream &operator<<(std::ostream &out_stream, const UnsubscribeMessage &message) {
    out_stream << "UnsubscribeMessage {" << "\n";
    out_stream << "  Header: \n";
    out_stream << "     Instance: " << message.getHeader().getInstance() << "\n";
    out_stream << "     Schema Type: " << schemaTypeToString(message.getHeader().getSchemaType())
               << "\n";
    if (message.getNodes().empty()) {
        out_stream << "  Nodes: None" << "\n";
    } else {
        out_stream << "  Nodes: " << "\n";
        for (const auto &node : message.getNodes()) {
            out_stream << "    " << node.getName() << "\n";
        }
    }
    out_stream << "}" << "\n";
    return out_stream;
}