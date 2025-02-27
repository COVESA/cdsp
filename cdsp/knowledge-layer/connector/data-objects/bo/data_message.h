#ifndef DATA_MESSAGE_H
#define DATA_MESSAGE_H

#include <ostream>
#include <vector>

#include "message_header.h"
#include "node.h"

class DataMessage {
   public:
    DataMessage(const MessageHeader& header, const std::vector<Node>& nodes);
    MessageHeader getHeader() const;
    std::vector<Node> getNodes() const;
    friend std::ostream& operator<<(std::ostream& os, const DataMessage& message);

   private:
    MessageHeader header_;
    std::vector<Node> nodes_;
};

#endif  // DATA_MESSAGE_H