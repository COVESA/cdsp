#ifndef MESSAGE_MODEL_CONFIG_H
#define MESSAGE_MODEL_CONFIG_H

#include <nlohmann/json.hpp>

#include "data_types.h"

using json = nlohmann::json;

void validateJsonFields(const json& config_json);
void loadModelConfig(const std::string& config_file, ModelConfig& model_config);

#endif  // MESSAGE_MODE_CONFIG_H