#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <vector>

#include "data_message_dto.h"
#include "data_types.h"
#include "get_message.h"
#include "status_message_dto.h"
#include "subscribe_message.h"

using json = nlohmann::json;

namespace MessageUtils {
void addMessageToQueue(const GetMessage& message, std::vector<json>& reply_messages_queue);

void addMessageToQueue(const SubscribeMessage& message, std::vector<json>& reply_messages_queue);

std::variant<DataMessageDto, StatusMessageDto, InternalErrorMessage> displayAndParseMessage(
    const std::string& message);
}  // namespace MessageUtils
#endif  // MESSAGE_UTILS_H