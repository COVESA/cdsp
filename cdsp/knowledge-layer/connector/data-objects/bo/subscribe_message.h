#ifndef SUBSCRIBE_MESSAGE_H
#define SUBSCRIBE_MESSAGE_H

#include "message_header.h"
#include "node.h"

class SubscribeMessage {
   public:
    SubscribeMessage(const MessageHeader& header, const std::vector<Node>& nodes);

    MessageHeader getHeader() const;
    std::vector<Node> getNodes() const;
    friend std::ostream& operator<<(std::ostream& os, const SubscribeMessage& message);

   private:
    MessageHeader header_;
    std::vector<Node> nodes_;
};

#endif  // SUBSCRIBE_MESSAGE_H
