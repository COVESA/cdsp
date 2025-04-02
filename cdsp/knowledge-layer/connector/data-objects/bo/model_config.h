#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include <map>
#include <string>
#include <vector>

#include "data_types.h"
#include "reasoner_settings.h"
#include "triple_assembler_helper.h"

class ModelConfig {
   public:
    ModelConfig(
        const std::map<SchemaType, std::vector<std::string>>& supported_data_points,
        const std::vector<std::pair<ReasonerSyntaxType, std::string>>& ontologies,
        const std::string& output_path,
        const std::vector<std::pair<RuleLanguageType, std::string>>& reasoner_rules,
        const std::vector<std::pair<ReasonerSyntaxType, std::string>>& validation_shapes,
        const TripleAssemblerHelper& triple_assembler_helper,
        const std::vector<std::pair<QueryLanguageType, std::string>>& reasoning_output_queries,
        const ReasonerSettings& reasoner_settings);

    virtual std::map<SchemaType, std::string> getObjectId() const;
    virtual std::map<SchemaType, std::vector<std::string>> getInputs() const;
    virtual std::vector<std::pair<ReasonerSyntaxType, std::string>> getOntologies() const;
    virtual std::string getOutput() const;
    virtual std::vector<std::pair<RuleLanguageType, std::string>> getReasonerRules() const;
    virtual std::vector<std::pair<ReasonerSyntaxType, std::string>> getValidationShapes() const;
    virtual TripleAssemblerHelper getQueriesTripleAssemblerHelper() const;
    virtual std::vector<std::pair<QueryLanguageType, std::string>> getReasoningOutputQueries()
        const;
    virtual ReasonerSettings getReasonerSettings() const;
    friend std::ostream& operator<<(std::ostream& os, const ModelConfig& config);

   private:
    std::map<SchemaType, std::vector<std::string>> inputs_;
    std::vector<std::pair<ReasonerSyntaxType, std::string>> ontologies_;
    std::string output_path_;
    std::vector<std::pair<RuleLanguageType, std::string>> reasoner_rules_;
    std::vector<std::pair<ReasonerSyntaxType, std::string>> validation_shapes_;
    ReasonerSettings reasoner_settings_;
    TripleAssemblerHelper triple_assembler_helper_;
    std::vector<std::pair<QueryLanguageType, std::string>> reasoning_output_queries_;
    std::map<SchemaType, std::string> object_ids_;
};

#endif  // MODEL_CONFIG_H