#ifndef MOCK_MODEL_CONFIG_H
#define MOCK_MODEL_CONFIG_H

#include <gmock/gmock.h>

#include "data_types.h"
#include "model_config.h"

class MockModelConfig : public ModelConfig {
   public:
    MockModelConfig()
        : ModelConfig(
              // Supported data points
              std::map<SchemaType, SchemaInputList>{
                  {SchemaType::VEHICLE, SchemaInputList{{"mocked_supported_data_points"}}}},
              // Ontologies
              std::vector<std::pair<ReasonerSyntaxType, std::string>>{},
              // Output path
              "mocked_output_path",
              // Reasoner rules
              std::vector<std::pair<RuleLanguageType, std::string>>{
                  {RuleLanguageType::DATALOG, "mocked_rule"}},
              // Validation shapes
              std::vector<std::pair<ReasonerSyntaxType, std::string>>{
                  {ReasonerSyntaxType::NQUADS, "mocked_validation_shapes"}},
              // Triple assembler helper
              TripleAssemblerHelper({{SchemaType::VEHICLE,
                                      {{QueryLanguageType::SPARQL, "mocked_query_object"},
                                       {QueryLanguageType::SPARQL, "mocked_query_data"}}}}),
              // Reasoning output queries
              std::vector<ReasoningOutputQuery>{
                  {QueryLanguageType::SPARQL, "mocked_reasoning_output_queries"}},
              // Reasoner settings
              ReasonerSettings(InferenceEngineType::RDFOX, ReasonerSyntaxType::TURTLE,
                               {SchemaType::VEHICLE}, true)) {}

    MOCK_METHOD((std::map<SchemaType, std::string>), getObjectId, (), (const, override));
    MOCK_METHOD((std::map<SchemaType, SchemaInputList>), getInputs, (), (const, override));
    MOCK_METHOD((std::vector<std::pair<ReasonerSyntaxType, std::string>>), getOntologies, (),
                (const, override));
    MOCK_METHOD((std::string), getOutput, (), (const, override));
    MOCK_METHOD((std::vector<std::pair<RuleLanguageType, std::string>>), getReasonerRules, (),
                (const, override));
    MOCK_METHOD((std::vector<std::pair<ReasonerSyntaxType, std::string>>), getValidationShapes, (),
                (const, override));
    MOCK_METHOD((TripleAssemblerHelper), getQueriesTripleAssemblerHelper, (), (const, override));
    MOCK_METHOD((std::vector<ReasoningOutputQuery>), getReasoningOutputQueries, (),
                (const, override));
    MOCK_METHOD((ReasonerSettings), getReasonerSettings, (), (const, override));
};

#endif  // MOCK_MODEL_CONFIG_H