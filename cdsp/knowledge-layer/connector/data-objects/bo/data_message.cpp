#include "data_message.h"

/**
 * @brief Constructs a DataMessage object with the given header and nodes.
 *
 * This constructor initializes a DataMessage object with the provided header and nodes.
 *
 * @param header The header of the data message.
 * @param nodes A vector of Node objects.
 * @throws std::invalid_argument if the nodes vector is empty for data messages.
 */
DataMessage::DataMessage(const MessageHeader& header, const std::vector<Node>& nodes)
    : header_(header), nodes_(nodes) {
    if (nodes_.empty()) {
        throw std::invalid_argument("Nodes vector cannot be empty");
    }
    for (const auto& node : nodes_) {
        if (!node.getValue().has_value()) {
            throw std::invalid_argument("Node value cannot be empty");
        }
    }
}

/**
 * @brief Retrieves the header of the data message.
 *
 * This function returns the header associated with the data message.
 *
 * @return MessageHeader The header of the data message.
 */
MessageHeader DataMessage::getHeader() const { return header_; }

/**
 * @brief Retrieves the list of nodes.
 *
 * This function returns a vector containing the nodes associated with the DataMessage.
 *
 * @return std::vector<Node> A vector of Node objects.
 */
std::vector<Node> DataMessage::getNodes() const { return nodes_; }

/**
 * @brief Overloads the << operator to print the DataMessage object.
 *
 * This function overloads the << operator to enable printing of DataMessage objects.
 *
 * @param os The output stream to write to.
 * @param message The DataMessage object to print.
 * @return std::ostream& The output stream with the DataMessage object printed to it.
 */
std::ostream& operator<<(std::ostream& os, const DataMessage& message) {
    os << "DataMessage {\n";
    os << "  ID: " << message.getHeader().getId() << "\n";
    os << "  Schema Type: " << schemaTypeToString(message.getHeader().getSchemaType()) << "\n";
    os << "  Nodes: [\n";

    for (const auto& node : message.getNodes()) {
        os << "    Node {\n";
        os << "      Name: " << node.getName() << "\n";
        os << "      Value: " << (node.getValue().has_value() ? node.getValue().value() : "null")
           << "\n";
        if (node.getMetadata().getGenerated()) {
            auto generated = node.getMetadata().getGenerated().value().time_since_epoch();
            os << "      Generated Time: " << generated.count() << " nanos\n";
        } else {
            os << "      Generated Time: null\n";
        }
        auto received = node.getMetadata().getReceived().time_since_epoch();
        os << "      Received Time: " << received.count() << " nanos\n";
        os << "    },\n";
    }

    os << "  ]\n";
    os << "}\n";
    return os;
}