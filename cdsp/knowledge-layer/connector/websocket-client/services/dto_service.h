#ifndef DTO_SERVICE_H
#define DTO_SERVICE_H

#include <nlohmann/json.hpp>

#include "data_message_dto.h"
#include "model_config_dto.h"
#include "set_message_dto.h"
#include "status_message_dto.h"

class DtoService {
   public:
    static DataMessageDTO parseDataJsonToDto(const nlohmann::json& json);
    static StatusMessageDTO parseStatusJsonToDto(const nlohmann::json& json);
    static ModelConfigDTO parseModelConfigJsonToDto(const nlohmann::json& json);

   private:
    static MetadataDTO parseMetadataJsonToDto(const nlohmann::json& metadata_json);
};

#endif  // DTO_SERVICE_H