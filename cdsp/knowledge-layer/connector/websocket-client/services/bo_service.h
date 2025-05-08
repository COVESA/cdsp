#ifndef BO_SERVICE_H
#define BO_SERVICE_H

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "data_types.h"
#include "get_message.h"
#include "set_message.h"
#include "subscribe_message.h"

class BoService {
   public:
    static SubscribeMessage createSubscribeMessage(const std::string& object_id,
                                                   SchemaType schema_type);

    static GetMessage createGetMessage(const std::string& object_id, SchemaType schema_type,
                                       const std::vector<std::string>& list_data_points);

    static std::vector<SetMessage> createSetMessage(
        const std::map<SchemaType, std::string>& object_id, const nlohmann::json& json);
};

#endif  // BO_SERVICE_H