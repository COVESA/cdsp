#ifndef STATUS_MESSAGE_CONVERTER_H
#define STATUS_MESSAGE_CONVERTER_H

#include "status_message.h"
#include "status_message_dto.h"

class StatusMessageConverter {
   public:
    static StatusMessage convert(const StatusMessageDTO& dto);
};

#endif  // STATUS_MESSAGE_CONVERTER_H