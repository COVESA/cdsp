#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <nlohmann/json.hpp>
#include <string>

#include "data_types.h"

using json = nlohmann::json;
namespace ModelConfigUtils {
void loadModelConfig(const std::string& config_file, ModelConfig& model_config);
}  // namespace ModelConfigUtils
#endif  // MODE_CONFIG_H