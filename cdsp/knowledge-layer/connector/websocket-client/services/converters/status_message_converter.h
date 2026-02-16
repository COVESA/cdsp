#ifndef STATUS_MESSAGE_CONVERTER_H
#define STATUS_MESSAGE_CONVERTER_H

#include "request_registry.h"
#include "status_message.h"
#include "status_message_dto.h"

class StatusMessageConverter {
   public:
    static StatusMessage convert(const StatusMessageDTO &dto, RequestRegistry &registry);
};

#endif  // STATUS_MESSAGE_CONVERTER_H