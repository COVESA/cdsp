#ifndef MOCK_MODEL_CONFIG_H
#define MOCK_MODEL_CONFIG_H

#include <gmock/gmock.h>

#include "model_config.h"

class MockModelConfig : public ModelConfig {
   public:
    MockModelConfig()
        : ModelConfig(
              std::map<SchemaType, std::vector<std::string>>{
                  {SchemaType::VEHICLE,
                   {"mocked_supported_data_points"}}},                    // supported_data_points
              std::vector<std::pair<ReasonerSyntaxType, std::string>>{},  // ontologies
              "mocked_output_path",                                       // output_path
              std::vector<std::pair<RuleLanguageType, std::string>>{
                  {RuleLanguageType::DATALOG, "mocked_rule"}},  // reasoner_rules
              std::vector<std::pair<ReasonerSyntaxType, std::string>>{
                  {ReasonerSyntaxType::NQUADS, "mocked_validation_shapes"}},  // validation_shapes
              TripleAssemblerHelper({{SchemaType::VEHICLE,
                                      {{QueryLanguageType::SPARQL, "mocked_query_object"},
                                       {QueryLanguageType::SPARQL, "mocked_query_data"}}}},
                                    "mocked_queries_output"),  // queries_config
              ReasonerSettings(InferenceEngineType::RDFOX, ReasonerSyntaxType::TURTLE,
                               {SchemaType::VEHICLE}  // reasoner_settings
                               )) {}

    MOCK_METHOD((std::map<SchemaType, std::string>), getObjectId, (), (const, override));
    MOCK_METHOD((std::map<SchemaType, std::vector<std::string>>), getInputs, (), (const, override));
    MOCK_METHOD((std::vector<std::pair<ReasonerSyntaxType, std::string>>), getOntologies, (),
                (const, override));
    MOCK_METHOD((std::string), getOutput, (), (const, override));
    MOCK_METHOD((std::vector<std::pair<RuleLanguageType, std::string>>), getReasonerRules, (),
                (const, override));
    MOCK_METHOD((std::vector<std::pair<ReasonerSyntaxType, std::string>>), getValidationShapes, (),
                (const, override));
    MOCK_METHOD((TripleAssemblerHelper), getQueriesConfig, (), (const, override));
    MOCK_METHOD((ReasonerSettings), getReasonerSettings, (), (const, override));
};

#endif  // MOCK_MODEL_CONFIG_H