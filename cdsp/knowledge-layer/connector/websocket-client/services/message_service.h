#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "data_message.h"
#include "data_message_dto.h"
#include "data_types.h"
#include "get_message.h"
#include "request_registry.h"
#include "set_message.h"
#include "status_message_dto.h"
#include "subscribe_message.h"
#include "unsubscribe_message.h"

using json = nlohmann::json;

class MessageService {
   public:
    static void createAndQueueSubscribeMessage(const std::string &object_id,
                                               const SchemaType &schema_type,
                                               const std::vector<std::string> &data_point_list,
                                               RequestRegistry &registry,
                                               std::vector<json> &reply_messages_queue);

    static void createAndQueueUnsubscribeMessage(const std::string &object_id,
                                                 const SchemaType &schema_type,
                                                 const std::vector<std::string> &data_point_list,
                                                 RequestRegistry &registry,
                                                 std::vector<json> &reply_messages_queue);

    static void createAndQueueSetMessage(const std::map<SchemaType, std::string> &object_id,
                                         const json &json_body, RequestRegistry &registry,
                                         std::vector<json> &reply_messages_queue,
                                         const std::string &origin_system_name);

    static std::optional<DataMessage> getDataOrProcessStatusFromMessage(const std::string &message,
                                                                        RequestRegistry &registry);

   private:
    static void addMessageToQueue(
        const std::variant<GetMessage, SetMessage, SubscribeMessage, UnsubscribeMessage> &message,
        RequestRegistry &registry, std::vector<json> &reply_messages_queue);

    static std::variant<DataMessageDTO, StatusMessageDTO> displayAndParseMessage(
        const std::string &message);
};
#endif  // MESSAGE_UTILS_H