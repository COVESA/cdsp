#include "model_config_fixture.h"

#include "model_config_utils.h"

ModelConfig ModelConfigFixture::getValidModelConfig() {
    ModelConfig model_config;
    ModelConfigUtils::loadModelConfig(
        std::string(PROJECT_ROOT) + "/symbolic-reasoner/examples/use-case/model/model_config.json",
        model_config);
    return model_config;
}