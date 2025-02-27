#ifndef DTO_TO_BO_H
#define DTO_TO_BO_H

#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

#include "data_message.h"
#include "data_message_dto.h"
#include "metadata.h"
#include "metadata_dto.h"
#include "status_message.h"
#include "status_message_dto.h"

class DtoToBo {
   public:
    static DataMessage convert(
        const DataMessageDto& dto,
        const std::map<SchemaType, std::vector<std::string>>& system_data_points);
    static StatusMessage convert(const StatusMessageDto& dto);

   private:
    static void parseNodes(const std::string& base_path, const nlohmann::json& data,
                           std::vector<Node>& nodes, const std::string& schema_collection,
                           const std::optional<MetadataDto>& metadata,
                           const std::vector<std::string>& supported_data_points);
    static Metadata extractBoMetadata(const std::optional<MetadataDto>& metadata,
                                      const std::string& path);

    static std::optional<std::chrono::system_clock::time_point> parseTimestamp(int64_t seconds,
                                                                               int64_t nanos);
};

#endif  // DTO_TO_BO_H