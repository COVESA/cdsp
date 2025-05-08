#include "model_config.h"

#include "helper.h"

/**
 * @brief Constructs a ModelConfig object with the specified parameters.
 *
 * @param inputs A map of SchemaType to a vector of strings representing the inputs.
 *               This map cannot be empty.
 * @param ontologies A vector of pairs, each containing a ReasonerSyntaxType and a string,
 *                   representing the ontologies.
 * @param output_path A string specifying the path for the output. This cannot be empty.
 * @param reasoner_rules A vector of pairs, each containing a RuleLanguageType and a string,
 *                       representing the reasoner rules. This cannot be empty.
 * @param validation_shapes A vector of pairs, each containing a ReasonerSyntaxType and a string,
 *                          representing the validation shapes. This cannot be empty.
 * @param triple_assembler_helper A TripleAssemblerHelper object containing the
 * queries to create triples. This cannot be empty.
 * @param reasoning_output_queries A vector of pairs, each containing a ReasonerSyntaxType and a
 * string, representing the query rules output. This cannot be empty.
 * @param reasoner_settings A ReasonerSettings object containing the settings for the reasoner.
 *                          It must have supported schema collections.
 *
 * @throws std::invalid_argument if the inputs map is empty.
 * @throws std::invalid_argument if the output path is empty.
 * @throws std::invalid_argument if the validation shapes are empty.
 * @throws std::invalid_argument if the reasoner rules are empty.
 * @throws std::invalid_argument if the triple_assembler_helper does not contain any
 * queries.
 * @throws std::invalid_argument if the reasoning_output_queries is empty.
 * @throws std::invalid_argument if the supported schema collections in reasoner_settings are empty.
 * @throws std::invalid_argument if the inputs map does not contain all supported schema
 * collections.
 * @throws std::invalid_argument if the triple_assembler_helper does not contain all
 * supported schema collections or a default query.
 */
ModelConfig::ModelConfig(
    const std::map<SchemaType, std::vector<std::string>>& inputs,
    const std::vector<std::pair<ReasonerSyntaxType, std::string>>& ontologies,
    const std::string& output_path,
    const std::vector<std::pair<RuleLanguageType, std::string>>& reasoner_rules,
    const std::vector<std::pair<ReasonerSyntaxType, std::string>>& validation_shapes,
    const TripleAssemblerHelper& triple_assembler_helper,
    const std::vector<std::pair<QueryLanguageType, std::string>>& reasoning_output_queries,
    const ReasonerSettings& reasoner_settings)
    : inputs_(inputs),
      ontologies_(ontologies),
      output_path_(output_path),
      reasoner_rules_(reasoner_rules),
      validation_shapes_(validation_shapes),
      triple_assembler_helper_(triple_assembler_helper),
      reasoning_output_queries_(reasoning_output_queries),
      reasoner_settings_(reasoner_settings) {
    if (inputs_.empty()) {
        throw std::invalid_argument("Inputs map cannot be empty");
    }
    if (output_path_.empty()) {
        throw std::invalid_argument("Output path cannot be empty");
    }
    if (validation_shapes_.empty()) {
        throw std::invalid_argument("Validation shapes cannot be empty");
    }
    if (reasoner_rules_.empty()) {
        throw std::invalid_argument("Reasoner rules cannot be empty");
    }
    if (triple_assembler_helper.getQueries().empty()) {
        throw std::invalid_argument(
            "Queries for the triple assembler helper cannot be empty. At least one query must be "
            "provided for each schema or default");
    }
    if (reasoning_output_queries_.empty()) {
        throw std::invalid_argument("Reasoning output queries cannot be empty");
    }
    if (reasoner_settings_.getSupportedSchemaCollections().empty()) {
        throw std::invalid_argument("Supported schema collections cannot be empty");
    } else {
        for (const auto& schema : reasoner_settings_.getSupportedSchemaCollections()) {
            if (inputs_.find(schema) == inputs_.end()) {
                throw std::invalid_argument(
                    "Inputs map must contain all supported schema collections");
            }
            if (triple_assembler_helper.getQueries().find(schema) ==
                triple_assembler_helper.getQueries().end()) {
                if (triple_assembler_helper.getQueries().find(SchemaType::DEFAULT) ==
                    triple_assembler_helper.getQueries().end()) {
                    throw std::invalid_argument(
                        "All supported schema collections must be in the queries map or there must "
                        "be a default query");
                }
            }

            // Add Object ID for each schema type
            std::string uppercase_schema_type = Helper::toUppercase(schemaTypeToString(schema));
            object_ids_[schema] = Helper::getEnvVariable(uppercase_schema_type + "_OBJECT_ID");
            if (object_ids_[schema].empty()) {
                throw std::invalid_argument("The environment variable for Object ID " +
                                            schemaTypeToString(schema) + " has not been set");
            }
        }
    }
}

/**
 * @brief Retrieves the object ID for the model configuration.
 *
 * This function returns a map where each key is a SchemaType and the corresponding value
 * is a string representing the object ID associated with that schema type.
 *
 * @return A map of SchemaType to a string representing the object ID.
 */
std::map<SchemaType, std::string> ModelConfig::getObjectId() const { return object_ids_; }

/**
 * @brief Retrieves the input configurations for the model.
 *
 * This function returns a map where each key is a SchemaType and the corresponding value
 * is a vector of strings representing the inputs associated with that schema type.
 *
 * @return A map of SchemaType to a vector of input strings.
 */
std::map<SchemaType, std::vector<std::string>> ModelConfig::getInputs() const { return inputs_; }

/**
 * @brief Retrieves the list of ontologies.
 *
 * This function returns a vector of pairs, where each pair consists of a
 * ReasonerSyntaxType and a corresponding string. The vector represents
 * the ontologies associated with the model configuration.
 *
 * @return A vector of pairs containing ReasonerSyntaxType and std::string.
 */
std::vector<std::pair<ReasonerSyntaxType, std::string>> ModelConfig::getOntologies() const {
    return ontologies_;
}

/**
 * @brief Retrieves the output path of the model configuration.
 *
 * @return A string representing the output path.
 */
std::string ModelConfig::getOutput() const { return output_path_; }

/**
 * @brief Retrieves the reasoner rules associated with the model configuration.
 *
 * @return A vector of pairs, where each pair consists of a RuleLanguageType and a string.
 *         The RuleLanguageType represents the type of rule language, and the string
 *         represents the rule itself.
 */
std::vector<std::pair<RuleLanguageType, std::string>> ModelConfig::getReasonerRules() const {
    return reasoner_rules_;
}

/**
 * @brief Retrieves the validation shapes for the model configuration.
 *
 * This function returns a vector of pairs, where each pair consists of a
 * ReasonerSyntaxType and a corresponding string. These pairs represent the
 * validation shapes associated with the model configuration.
 *
 * @return A vector of pairs containing ReasonerSyntaxType and string,
 * representing the validation shapes.
 */
std::vector<std::pair<ReasonerSyntaxType, std::string>> ModelConfig::getValidationShapes() const {
    return validation_shapes_;
}

/**
 * @brief Retrieves the queries used by the triple assembler.
 *
 * This function returns a TripleAssemblerHelper object containing the queries used by the
 * triple assembler to construct the triples.
 *
 * @return TripleAssemblerHelper The TripleAssemblerHelper object containing the queries.
 */
TripleAssemblerHelper ModelConfig::getQueriesTripleAssemblerHelper() const {
    return triple_assembler_helper_;
}

/**
 * @brief Retrieves the queries rules output configuration.
 *
 * This function returns a vector of pairs, where each pair consists of a
 * QueryLanguageType and a corresponding string. These pairs represent the
 * queries used to get the rules output associated with the model configuration.
 *
 * @return A vector of pairs containing QueryLanguageType and string,
 * representing the queries of the rules output.
 */
std::vector<std::pair<QueryLanguageType, std::string>> ModelConfig::getReasoningOutputQueries()
    const {
    return reasoning_output_queries_;
}

/**
 * @brief Retrieves the current reasoner settings.
 *
 * This function returns the reasoner settings associated with the model configuration.
 *
 * @return ReasonerSettings The current reasoner settings.
 */
ReasonerSettings ModelConfig::getReasonerSettings() const { return reasoner_settings_; }

/**
 * @brief Overloads the << operator to print the ModelConfig object.
 *
 * This function overloads the << operator to enable printing of ModelConfig objects.
 *
 * @param os The output stream to write to.
 * @param config The ModelConfig object to print.
 * @return std::ostream& The output stream with the ModelConfig object printed to it.
 */
std::ostream& operator<<(std::ostream& os, const ModelConfig& config) {
    os << "ModelConfig {\n";
    os << "  Object ID: {\n";
    for (const auto& [schema, object_id] : config.object_ids_) {
        os << "    " << schemaTypeToString(schema) << ": " << object_id << "\n";
    }
    os << "  }\n";
    os << "  Inputs: {\n";
    for (const auto& [schema, data_points] : config.getInputs()) {
        os << "    " << schemaTypeToString(schema) << ": [\n";
        for (const auto& data_point : data_points) {
            os << "      " << data_point << ",\n";
        }
        os << "    ]\n";
    }
    os << "  }\n";
    os << "  Ontologies: [\n";
    for (const auto& [syntax, content] : config.getOntologies()) {
        os << "    {\n";
        os << "      Syntax: " << reasonerSyntaxTypeToContentType(syntax) << ",\n";
        os << "      Content: " << content << "\n";
        os << "    }\n";
    }
    os << "  ]\n";
    os << "  Output: " << config.getOutput() << "\n";
    os << "  Reasoner Rules: [\n";
    for (const auto& [rule_type, content] : config.getReasonerRules()) {
        os << "   {\n";
        os << "    Rule Type: " << ruleLanguageTypeToContentType(rule_type) << ",\n";
        os << "    Content: " << content << "\n";
        os << "   }\n";
    }
    os << "  ]\n";
    os << "  Validation Shapes: [\n";
    for (const auto& [syntax, content] : config.getValidationShapes()) {
        os << "    {\n";
        os << "      Syntax: " << reasonerSyntaxTypeToContentType(syntax) << ",\n";
        os << "      Content: " << content << "\n";
        os << "    }\n";
    }
    os << "  ]\n";
    os << "  Queries Config: \n";
    for (const auto& [schema, query_pair] : config.getQueriesTripleAssemblerHelper().getQueries()) {
        if (schema == SchemaType::DEFAULT) {
            os << "    default: {\n";
        } else {
            os << "    " << schemaTypeToString(schema) << ": {\n";
        }
        os << "      Data Property: \n";
        os << "         {\n";
        os << "           Query Type: "
           << queryLanguageTypeToContentType(query_pair.data_property.first) << "\n";
        os << "           Query: " << query_pair.data_property.second << "\n";
        os << "         }\n";
        os << "      Object Property: \n";
        os << "         {\n";
        os << "           Query Type: "
           << queryLanguageTypeToContentType(query_pair.object_property.first) << "\n";
        os << "           Query: " << query_pair.object_property.second << "\n";
        os << "         }\n";
        os << "    }\n";
    }
    os << "  Queries Rules Output: [\n";
    for (const auto& [syntax, content] : config.getReasoningOutputQueries()) {
        os << "    {\n";
        os << "      Query Type: " << queryLanguageTypeToContentType(syntax) << ",\n";
        os << "      Content: " << content << "\n";
        os << "    }\n";
    }
    os << "  ]\n";
    os << "  Reasoner Settings:\n";
    os << "    Inference Engine: "
       << inferenceEngineTypeToString(config.getReasonerSettings().getInferenceEngine()) << "\n";
    os << "    Output Format: "
       << reasonerSyntaxTypeToContentType(config.getReasonerSettings().getOutputFormat()) << "\n";
    os << "    Supported Schema Collections: [\n";
    for (const auto& schema : config.getReasonerSettings().getSupportedSchemaCollections()) {
        os << "      " << schemaTypeToString(schema) << ",\n";
    }
    os << "    ]\n";
    os << "}";
    return os;
}