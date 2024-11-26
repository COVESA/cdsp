#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <vector>

#include "data_types.h"

using json = nlohmann::json;

json createMessageHeader(const std::string& type, const std::string& tree, const std::string& id,
                         const std::string& uuid);

void createSubscription(const std::string& uuid, const std::string& oid, const std::string& tree,
                        std::vector<json>& reply_messages_queue);

void createReadMessage(const std::string& uuid, const std::string& tree, const std::string& oid,
                       const std::vector<std::string>& data_points,
                       std::vector<json>& reply_messages_queue);

CategoryMessage parseCategoryMessage(const json& json_message);
ErrorMessage parseErrorMessage(const json& json_message);

std::string nodeValueToString(const json& json_value);

DataMessage parseSuccessMessage(const json& json_message);

std::variant<DataMessage, ErrorMessage, CategoryMessage> displayAndParseMessage(
    const std::string& message);

#endif  // MESSAGE_UTILS_H