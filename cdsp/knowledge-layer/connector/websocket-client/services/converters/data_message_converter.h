#ifndef DATA_MESSAGE_CONVERTER_H
#define DATA_MESSAGE_CONVERTER_H

#include "data_message.h"
#include "data_message_dto.h"

class DataMessageConverter {
   public:
    static DataMessage convert(
        const DataMessageDTO& dto,
        const std::map<SchemaType, std::vector<std::string>>& system_data_points);

   private:
    static void parseNodes(const std::string& base_path, const nlohmann::json& data,
                           std::vector<Node>& nodes, const std::string& schema_collection,
                           const std::optional<MetadataDTO>& metadata,
                           const std::vector<std::string>& supported_data_points);
    static Metadata extractBoMetadata(const std::optional<MetadataDTO>& metadata,
                                      const std::string& path);
};

#endif  // DATA_MESSAGE_CONVERTER_H