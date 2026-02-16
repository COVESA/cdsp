#ifndef BO_TO_DTO_H
#define BO_TO_DTO_H

#include <vector>

#include "get_message.h"
#include "get_message_dto.h"
#include "set_message.h"
#include "set_message_dto.h"
#include "subscribe_message.h"
#include "subscribe_message_dto.h"
#include "unsubscribe_message.h"
#include "unsubscribe_message_dto.h"

class BoToDto {
   public:
    static std::vector<GetMessageDTO> convert(const GetMessage &b_obj);

    static std::vector<SubscribeMessageDTO> convert(const SubscribeMessage &b_obj);

    static std::vector<UnsubscribeMessageDTO> convert(const UnsubscribeMessage &b_obj);

    static std::vector<SetMessageDTO> convert(const SetMessage &b_obj);
};

#endif  // BO_TO_DTO_H
