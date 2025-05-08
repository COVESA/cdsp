#ifndef BO_TO_DTO_H
#define BO_TO_DTO_H

#include <vector>

#include "data_types.h"
#include "get_message.h"
#include "get_message_dto.h"
#include "set_message.h"
#include "set_message_dto.h"
#include "subscribe_message.h"
#include "subscribe_message_dto.h"

class BoToDto {
   public:
    static std::vector<GetMessageDTO> convert(const GetMessage& bo);
    static std::vector<SubscribeMessageDTO> convert(const SubscribeMessage& bo);

    static SetMessageDTO convert(const SetMessage& bo);
};

#endif  // BO_TO_DTO_H
