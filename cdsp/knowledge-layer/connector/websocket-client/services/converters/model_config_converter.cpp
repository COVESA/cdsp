#include "model_config_converter.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "triple_assembler_helper.h"

// Define the static constant variable
const std::string ModelConfigConverter::INPUT_SUFFIX = "_data";

ModelConfigConverter::ModelConfigConverter(std::shared_ptr<IFileHandler> file_handler)
    : file_handler_(std::move(file_handler)) {}

/**
 * @brief Converts a ModelConfigDTO object to a ModelConfig object.
 *
 * This function takes a ModelConfigDTO object and converts its components into a
 * ModelConfig object. It processes various elements such as inputs, ontologies,
 * output path, rules files, SHACL shapes, queries to create triples and query rules, and reasoner
 * settings from the DTO and uses them to construct a new ModelConfig object.
 *
 * @param dto The ModelConfigDTO object containing the data to be converted.
 * @return ModelConfig A new ModelConfig object created from the DTO data.
 */
ModelConfig ModelConfigConverter::convert(const ModelConfigDTO& dto) {
    auto reasoner_settings = convertReasonerSettings(dto.reasoner_settings);
    auto inputs = getInputsFromDto(dto.inputs);
    auto ontologies = getReasonerSyntaxTypeAndContent(dto.ontologies);
    auto output_path = getFullModelConfigPath(dto.output);
    auto rules_files = getReasonerRules(dto.rules);
    auto validation_shapes = getReasonerSyntaxTypeAndContent(dto.shacl_shapes);
    auto triple_assembler_helper =
        convertTripleAssemblerHelper(dto.queries.triple_assembler_helper);
    auto reasoning_output_queries =
        getReasoningOutputQueries(dto.queries.reasoning_output_queries_path);

    return ModelConfig(inputs, ontologies, output_path, rules_files, validation_shapes,
                       triple_assembler_helper, reasoning_output_queries, reasoner_settings);
}

/**
 * Converts a ReasonerSettingsDTO object to a ReasonerSettings business object.
 *
 * @param dto The ReasonerSettingsDTO object containing the data to be converted.
 * @return A ReasonerSettings object initialized with the converted data.
 */
ReasonerSettings ModelConfigConverter::convertReasonerSettings(const ReasonerSettingsDTO& dto) {
    if (dto.inference_engine.empty() || dto.output_format.empty()) {
        throw std::invalid_argument("Reasoner settings fields cannot be empty");
    }
    InferenceEngineType inference_engine = stringToInferenceEngineType(dto.inference_engine);
    ReasonerSyntaxType output_format = reasonerOutputFormatToReasonerSyntaxType(dto.output_format);
    std::vector<SchemaType> supported_schema_collections;
    for (const auto& schema : dto.supported_schema_collections) {
        supported_schema_collections.push_back(stringToSchemaType(schema));
    }
    bool is_ai_reasoner_inference_results = dto.is_ai_reasoner_inference_results;
    return ReasonerSettings(inference_engine, output_format, supported_schema_collections,
                            is_ai_reasoner_inference_results);
}

/**
 * Converts a map of queries to create triples from a ModelConfigDTO object to a
 * TripleAssemblerHelper object.
 *
 * This function processes the queries to create triples from the DTO and converts them
 * into a TripleAssemblerHelper object.
 *
 * @param queries_dto The map of queries to create triples from the ModelConfigDTO object.
 * @return A TripleAssemblerHelper object containing the converted queries.
 */
TripleAssemblerHelper ModelConfigConverter::convertTripleAssemblerHelper(
    const std::map<std::string, std::vector<std::string>>& queries_dto) {
    std::map<SchemaType, TripleAssemblerHelper::QueryPair> queries;
    for (const auto& [collection, query_list] : queries_dto) {
        SchemaType schema_type;
        if (collection == "default") {
            schema_type = SchemaType::DEFAULT;
        } else {
            schema_type = stringToSchemaType(collection);
        }
        auto query_pair = getQueriesToCreateTriples(query_list);
        if (query_pair.data_property.second.empty() || query_pair.object_property.second.empty()) {
            if (schema_type == SchemaType::DEFAULT) {
                std::cout << " - Default queries are not provided in the model config."
                          << std::endl;
                continue;
            }
            throw std::invalid_argument("Failed to read data and object properties for `" +
                                        collection + "` collection from the model config");
        }
        queries[schema_type] = query_pair;
    }

    return queries;
}

/**
 * Retrieves a pair of queries to create triples based on the provided list of query file paths.
 *
 * This function processes each file path in the given list to determine its type and content.
 * It identifies files related to data properties and object properties, reads their content,
 * and pairs them with their respective query language types.
 *
 * @param query_path_list A vector of strings representing the paths to query files.
 * @return A QueryPair containing pairs of query language types and their corresponding file
 * contents for data properties and object properties.
 */
TripleAssemblerHelper::QueryPair ModelConfigConverter::getQueriesToCreateTriples(
    const std::vector<std::string>& query_path_list) {
    std::pair<QueryLanguageType, std::string> data_property;
    std::pair<QueryLanguageType, std::string> object_property;
    for (const auto& file_path : query_path_list) {
        auto file_extension = std::filesystem::path(file_path).extension().string();
        if (file_path.find("data_property") != std::string::npos) {
            data_property = getQueryLanguageTypeAndContent(file_path);
        } else if (file_path.find("object_property") != std::string::npos) {
            object_property = getQueryLanguageTypeAndContent(file_path);
        }
    }
    return TripleAssemblerHelper::QueryPair{data_property, object_property};
}

/**
 * @brief Converts a vector of query rules output file paths into a vector of pairs
 *        containing the query language type and the corresponding file content.
 *
 * This function iterates over each file path in the provided vector, retrieves the
 * full path of the model configuration file, reads its content, determines the
 * query language type based on the file extension, and stores the results in a
 * vector of pairs.
 *
 * @param reasoning_output_queries_path A vector of strings representing the file paths of
 *                           query rules output.
 * @return A vector of pairs, where each pair consists of a QueryLanguageType
 *         and the corresponding file content as a string.
 */
std::vector<std::pair<QueryLanguageType, std::string>>
ModelConfigConverter::getReasoningOutputQueries(const std::string& reasoning_output_queries_path) {
    try {
        std::vector<std::string> file_list =
            file_handler_->readDirectory(getFullModelConfigPath(reasoning_output_queries_path));
        std::vector<std::pair<QueryLanguageType, std::string>> reasoning_output_queries;
        for (const auto& file_path : file_list) {
            reasoning_output_queries.push_back(
                getQueryLanguageTypeAndContent(reasoning_output_queries_path + "/" + file_path));
        }
        return reasoning_output_queries;
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to read the query rules output directory: " << e.what() << std::endl;
        return {};
    }
}

/**
 * Retrieves the query language type and content from a model configuration file.
 *
 * This function constructs the full path to the model configuration file using the provided
 * file path, reads the content of the file, and determines the query language type based
 * on the file extension. It returns a pair containing the query language type and the
 * content of the file.
 *
 * @param file_path The relative or absolute path to the model configuration file.
 * @return A pair consisting of the QueryLanguageType and the content of the file as a string.
 */
std::pair<QueryLanguageType, std::string> ModelConfigConverter::getQueryLanguageTypeAndContent(
    const std::string& file_path) {
    try {
        auto full_path = getFullModelConfigPath(file_path);
        auto file_content = file_handler_->readFile(full_path);
        auto query_type =
            fileExtensionToQueryLanguageType(std::filesystem::path(full_path).extension().string());
        return std::make_pair(query_type, file_content);
    } catch (const std::exception& e) {
        std::cerr << "Failed to read the query file " << file_path << ": " << e.what() << std::endl;
        return std::make_pair(QueryLanguageType::SPARQL, "");
    }
}

/**
 * Converts a map of input data points from a ModelConfigDTO object to a map of SchemaType and
 * vector of supported data points.
 *
 * @param inputs The map of input data points from the ModelConfigDTO object.
 * @return A map of SchemaType and vector of strings containing the converted data.
 */
std::map<SchemaType, std::vector<std::string>> ModelConfigConverter::getInputsFromDto(
    const std::map<std::string, std::string>& inputs) {
    std::map<SchemaType, std::vector<std::string>> inputs_map;
    for (const auto& [collection, data] : inputs) {
        auto collection_name = collection.substr(0, collection.find(INPUT_SUFFIX));
        auto schema_type = stringToSchemaType(collection_name);
        auto data_points = getSupportedDataPoints(data);
        inputs_map[schema_type] = data_points;
    }
    return inputs_map;
}

/**
 * @brief Converts a list of file paths to a vector of RDF syntax types and their corresponding file
 * contents.
 *
 * This function takes a list of file paths, reads the content of each file, determines the RDF
 * syntax type, and returns a vector of pairs containing the RDF syntax type and the file content.
 *
 * @param file_list A vector of strings representing the file paths to be processed.
 * @return A vector of pairs, where each pair consists of an ReasonerSyntaxType and a string
 * representing the file content.
 */
std::vector<std::pair<ReasonerSyntaxType, std::string>>
ModelConfigConverter::getReasonerSyntaxTypeAndContent(const std::vector<std::string>& file_list) {
    std::vector<std::pair<ReasonerSyntaxType, std::string>> mapping_result;
    for (const auto& file_path : file_list) {
        try {
            auto full_path = getFullModelConfigPath(file_path);
            auto file_content = file_handler_->readFile(full_path);
            auto rdf_syntax = getReasonerSyntaxTypeFromFile(full_path);
            mapping_result.push_back(std::make_pair(rdf_syntax, file_content));
        } catch (const std::exception& e) {
            std::cerr << "Failed to read the reasoning file " << file_path << ": " << e.what()
                      << std::endl;
        }
    }
    return mapping_result;
}

/**
 * Retrieves a list of supported data points from a specified configuration file.
 *
 * This function reads a file containing data points and returns them as a vector of strings.
 * It throws a runtime error if the file cannot be opened.
 *
 * @param file_name The name of the file containing the data points.
 * @return A vector of strings, each representing a supported data point.
 * @throws std::runtime_error If the file cannot be opened.
 */
std::vector<std::string> ModelConfigConverter::getSupportedDataPoints(std::string file_name) {
    try {
        std::vector<std::string> required_data;
        std::string root = getFullModelConfigPath(file_name);
        auto file_content = file_handler_->readFile(root);
        std::istringstream file(file_content);
        std::string line;
        while (std::getline(file, line)) {
            required_data.push_back(line);
        }
        return required_data;
    } catch (const std::exception& e) {
        std::cerr << "Failed to read the file " << file_name << ": " << e.what() << std::endl;
        return {};
    }
}

/**
 * @brief Retrieves reasoner rules from a list of file paths and returns them with their rule
 * language types.
 *
 * This function processes a list of file paths, reads the content of each file, and determines the
 * rule language type based on the file extension. It returns a vector of pairs, where each pair
 * consists of the rule language type and the file content as a string. If a file cannot be read, an
 * error message is printed to the console.
 *
 * @param file_list A vector of strings representing the file paths of the reasoner rules.
 * @return std::vector<std::pair<RuleLanguageType, std::string>> A vector of pairs, each containing
 * the rule language type and the content of a reasoner rule file.
 */
std::vector<std::pair<RuleLanguageType, std::string>> ModelConfigConverter::getReasonerRules(
    const std::vector<std::string>& file_list) {
    std::vector<std::pair<RuleLanguageType, std::string>> reasoner_rules;
    for (const auto& file_path : file_list) {
        try {
            auto file_extension = std::filesystem::path(file_path).extension().string();
            auto rule_type = fileExtensionToRuleLanguageType(file_extension);
            auto full_path = getFullModelConfigPath(file_path);
            auto file_content = file_handler_->readFile(full_path);
            reasoner_rules.push_back(std::make_pair(rule_type, file_content));
        } catch (const std::exception& e) {
            std::cout << " - Failed to read the file " << file_path << ": " << e.what()
                      << std::endl;
        }
    }
    return reasoner_rules;
}

/**
 * @brief Determines the RDF syntax type based on the file extension.
 *
 * This function extracts the file extension from the given file path and
 * converts it to an ReasonerSyntaxType using the `fileExtensionToReasonerSyntaxType` function.
 *
 * @param file_path The path to the file whose RDF syntax type is to be determined.
 * @return ReasonerSyntaxType The RDF syntax type corresponding to the file extension.
 */
ReasonerSyntaxType ModelConfigConverter::getReasonerSyntaxTypeFromFile(
    const std::string& file_path) {
    auto file_extension = std::filesystem::path(file_path).extension().string();
    return fileExtensionToReasonerSyntaxType(file_extension);
}

/**
 * @brief Constructs the full path to a model configuration file.
 *
 * This function appends the provided model configuration file name to the
 * predefined path where model files are stored, returning the complete path.
 *
 * @param model_config_file The name of the model configuration file.
 * @return std::string The full path to the model configuration file.
 */
std::string ModelConfigConverter::getFullModelConfigPath(const std::string& model_config_file) {
    if (model_config_file.empty()) {
        return "";  // Failure will provide from the BO for each specific case
    }
    return PATH_TO_MODEL_FILES + model_config_file;
}