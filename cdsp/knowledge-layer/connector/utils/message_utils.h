#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <vector>

#include "data_types.h"

using json = nlohmann::json;

namespace MessageUtils {
void createSubscription(const std::string& uuid, const std::string& oid, const std::string& tree,
                        std::vector<json>& reply_messages_queue);

void createReadMessage(const std::string& uuid, const std::string& tree, const std::string& oid,
                       const std::vector<std::string>& data_points,
                       std::vector<json>& reply_messages_queue);

std::variant<DataMessage, ErrorMessage, CategoryMessage> displayAndParseMessage(
    const std::string& message);
}  // namespace MessageUtils
#endif  // MESSAGE_UTILS_H