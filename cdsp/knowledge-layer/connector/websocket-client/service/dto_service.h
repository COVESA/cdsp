#ifndef DTO_SERVICE_H
#define DTO_SERVICE_H

#include <nlohmann/json.hpp>

#include "data_message_dto.h"
#include "status_message_dto.h"

class DtoService {
   public:
    static DataMessageDto parseDataDto(const nlohmann::json& json);
    static StatusMessageDto parseStatusDto(const nlohmann::json& json);

   private:
    static MetadataDto parseMetadataDto(const nlohmann::json& metadata_json);
};

#endif  // DTO_SERVICE_H