#ifndef SET_MESSAGE_H
#define SET_MESSAGE_H

#include "message_header.h"
#include "node.h"

class SetMessage {
   public:
    SetMessage(const MessageHeader& header, const std::vector<Node>& nodes);

    MessageHeader getHeader() const;
    std::vector<Node> getNodes() const;
    friend std::ostream& operator<<(std::ostream& os, const SetMessage& message);

   private:
    MessageHeader header_;
    std::vector<Node> nodes_;
};

#endif  // SET_MESSAGE_H