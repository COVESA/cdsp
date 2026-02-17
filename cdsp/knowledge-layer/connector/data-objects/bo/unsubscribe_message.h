#ifndef UNSUBSCRIBE_MESSAGE_H
#define UNSUBSCRIBE_MESSAGE_H

#include "message_header.h"
#include "node.h"

class UnsubscribeMessage {
   public:
    UnsubscribeMessage(MessageHeader header, std::vector<Node> nodes);

    [[nodiscard]] MessageHeader getHeader() const;
    [[nodiscard]] std::vector<Node> getNodes() const;
    friend std::ostream &operator<<(std::ostream &out_stream, const UnsubscribeMessage &message);

   private:
    MessageHeader header_;
    std::vector<Node> nodes_;
};

#endif  // UNSUBSCRIBE_MESSAGE_H