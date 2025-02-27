#ifndef BO_TO_DTO_H
#define BO_TO_DTO_H

#include <vector>

#include "data_types.h"
#include "get_message.h"
#include "get_message_dto.h"
#include "subscribe_message.h"
#include "subscribe_message_dto.h"

class BoToDto {
   public:
    static std::vector<GetMessageDto> convert(const GetMessage& bo,
                                              const std::optional<MessageStructureFormat>& format);
    static std::vector<SubscribeMessageDto> convert(
        const SubscribeMessage& bo, const std::optional<MessageStructureFormat>& format);
};

#endif  // BO_TO_DTO_H
