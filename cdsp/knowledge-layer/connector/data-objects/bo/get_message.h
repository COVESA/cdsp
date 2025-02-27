#ifndef GET_MESSAGE_H
#define GET_MESSAGE_H

#include "message_header.h"
#include "node.h"

class GetMessage {
   public:
    GetMessage(const MessageHeader& header, const std::vector<Node>& nodes);

    MessageHeader getHeader() const;
    std::vector<Node> getNodes() const;
    friend std::ostream& operator<<(std::ostream& os, const GetMessage& message);

   private:
    MessageHeader header_;
    std::vector<Node> nodes_;
};

#endif  // GET_MESSAGE_H
