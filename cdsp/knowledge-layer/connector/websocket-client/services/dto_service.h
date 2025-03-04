#ifndef DTO_SERVICE_H
#define DTO_SERVICE_H

#include <nlohmann/json.hpp>

#include "data_message_dto.h"
#include "model_config_dto.h"
#include "status_message_dto.h"

class DtoService {
   public:
    static DataMessageDTO parseDataDto(const nlohmann::json& json);
    static StatusMessageDTO parseStatusDto(const nlohmann::json& json);
    static ModelConfigDTO parseModelConfigDto(const nlohmann::json& json);

   private:
    static MetadataDTO parseMetadataDTO(const nlohmann::json& metadata_json);
};

#endif  // DTO_SERVICE_H