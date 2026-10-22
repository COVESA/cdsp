#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

#include "globals.h"
#include "system_configuration_service.h"

/**
 * Loads the use cases shipped in `symbolic-reasoner/examples/use-case/` from disk. Missing SHACL
 * or ontology files are only logged by the loader, so the test checks that every file listed in
 * `model_config.json`, including the ones in `shared/`, was actually read.
 */
class UseCaseConfigIntegrationTest : public ::testing::TestWithParam<std::string> {
   protected:
    void SetUp() override { setenv("VEHICLE_OBJECT_ID", "VINABCD1234567890", 1); }
};

TEST_P(UseCaseConfigIntegrationTest, LoadsAllFilesOfTheUseCase) {
    setPathToUseCases("/symbolic-reasoner/examples/use-case/" + GetParam() + "/");

    const ModelConfig model_config = SystemConfigurationService::loadModelConfig(
        getProjectRoot() + getPathToUseCases() + "model_config.json");

    EXPECT_FALSE(model_config.getInputs().at(SchemaType::VEHICLE).subscribe.empty());
    EXPECT_EQ(model_config.getOntologies().size(), 1);
    EXPECT_EQ(model_config.getValidationShapes().size(), 2);
    EXPECT_EQ(model_config.getReasonerRules().size(), 1);
    EXPECT_EQ(model_config.getQueriesTripleAssemblerHelper().getQueries().count(SchemaType::VEHICLE),
              1);
    EXPECT_FALSE(model_config.getReasoningOutputQueries().empty());
}

INSTANTIATE_TEST_SUITE_P(ShippedUseCases, UseCaseConfigIntegrationTest,
                         ::testing::Values("rdfox_model", "rdf4j_model"));
