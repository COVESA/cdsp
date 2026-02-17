#ifndef DATA_MESSAGE_CONVERTER_H
#define DATA_MESSAGE_CONVERTER_H

#include "data_message.h"
#include "data_message_dto.h"
#include "request_registry.h"

struct MetadataPathComponents {
    std::string base_path;
    std::string node_name;
};

class DataMessageConverter {
   public:
    static DataMessage convert(const DataMessageDTO &dto, RequestRegistry &registry);

   private:
    static void parseNodes(const std::string &base_path, const nlohmann::json &data,
                           std::vector<Node> &nodes, const std::string &schema_collection);
    static std::vector<Node> includeMetadata(const std::string &schema_collection,
                                             const std::string &base_path,
                                             const std::optional<MetadataDTO> &metadata_dto,
                                             const std::vector<Node> &nodes);
    static std::string normalizeMetadataPath(const std::string &schema_collection,
                                             const MetadataPathComponents &components);
    static Metadata findMetadata(
        const std::unordered_map<std::string, MetadataDTO::NodeMetadata> &nodes,
        const std::string &metadata_path);
};

#endif  // DATA_MESSAGE_CONVERTER_H