#ifndef SYSTEM_CONFIGURATION_SERVICE_H
#define SYSTEM_CONFIGURATION_SERVICE_H

#include <optional>
#include <string>

#include "data_types.h"
#include "model_config.h"

class SystemConfigurationService {
   public:
    static SystemConfig loadSystemConfig(
        std::optional<std::string> ws_server_host, std::optional<std::string> ws_server_port,
        std::optional<std::string> ws_server_target,
        std::optional<std::string> reasoner_server_host,
        std::optional<std::string> reasoner_server_port,
        std::optional<std::string> reasoner_server_auth_base64,
        std::optional<std::string> reasoner_server_data_store_name);
    static ModelConfig loadModelConfig(const std::string& config_file);
};

#endif  // SYSTEM_CONFIGURATION_SERVICE_H