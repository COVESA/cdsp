#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "data_message.h"
#include "data_message_dto.h"
#include "data_types.h"
#include "get_message.h"
#include "model_config.h"
#include "set_message.h"
#include "status_message_dto.h"
#include "subscribe_message.h"

using json = nlohmann::json;

class MessageService {
   public:
    MessageService(const std::map<SchemaType, std::vector<std::string>>& system_data_point);

    void addMessageToQueue(const std::variant<GetMessage, SubscribeMessage, SetMessage>& message,
                           std::vector<json>& reply_messages_queue);

    std::optional<DataMessage> getDataMessageOrLogStatus(const std::string& message);

   private:
    std::map<SchemaType, std::vector<std::string>> system_data_point_;

    std::variant<DataMessageDTO, StatusMessageDTO> displayAndParseMessage(
        const std::string& message);
};
#endif  // MESSAGE_UTILS_H