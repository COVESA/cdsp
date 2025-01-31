#include "model_config_utils.h"

#include <fstream>
#include <iostream>

#include "helper.h"

namespace {
/**
 * @brief Validates the presence of required fields in a JSON configuration.
 *
 * This function checks for the existence of specific top-level fields and nested fields
 * within the provided JSON object. If any required field is missing, it throws a runtime error
 * with a descriptive message indicating which field is missing.
 *
 * @param config_json The JSON object representing the configuration to be validated.
 *
 * @throws std::runtime_error If any required field is missing in the JSON configuration.
 */
void validateJsonFields(const json& config_json) {
    const std::string generic_error_message =
        "Error in the model_config.json file. Missing required field";
    // Required top-level fields
    if (!config_json.contains("inputs")) {
        throw std::runtime_error(generic_error_message + ": 'inputs'");
    }
    if (!config_json.contains("ontologies")) {
        throw std::runtime_error(generic_error_message + ": 'ontologies'");
    }
    if (!config_json.contains("output")) {
        throw std::runtime_error(generic_error_message + ": 'output'");
    }
    if (!config_json.contains("queries")) {
        throw std::runtime_error(generic_error_message + ": 'queries'");
    }
    if (!config_json.contains("rules")) {
        throw std::runtime_error(generic_error_message + ": 'rules'");
    }
    if (!config_json.contains("shacl")) {
        throw std::runtime_error(generic_error_message + ": 'shacl'");
    }
    if (!config_json.contains("reasoner_settings")) {
        throw std::runtime_error(generic_error_message + " 'reasoner_settings'");
    }

    // Validate structure inside "queries"
    const auto& queries = config_json["queries"];
    if (!queries.contains("triple_assembler_helper")) {
        throw std::runtime_error(generic_error_message +
                                 " in 'queries': 'triple_assembler_helper'");
    }
    if (!queries.contains("output")) {
        throw std::runtime_error(generic_error_message + " in 'queries': 'output'");
    }

    // Validate structure inside "reasoner_settings"
    const auto& reasoner_settings = config_json["reasoner_settings"];
    if (!reasoner_settings.contains("inference_engine")) {
        throw std::runtime_error(generic_error_message +
                                 " in 'reasoner_settings': 'inference_engine'");
    }
    if (!reasoner_settings.contains("output_format")) {
        throw std::runtime_error(generic_error_message +
                                 " in 'reasoner_settings': 'output_format'");
    }
    if (!reasoner_settings.contains("supported_tree_types")) {
        throw std::runtime_error(generic_error_message +
                                 " in 'reasoner_settings': 'supported_tree_types'");
    }
}

std::string createConfigPath(const std::string& config_file) {
    return std::string(PROJECT_ROOT) + "/symbolic-reasoner/examples/use-case/model/" + config_file;
}
}  // namespace

namespace ModelConfigUtils {
/**
 * @brief Reads a file and returns a list of required data points.
 *
 * This function constructs the full path to the file using the provided file name
 * and a predefined project root directory. It then reads the file line by line,
 * storing each line as an element in a vector of strings.
 *
 * @param file_name The name of the file to read.
 * @return A vector of strings, each representing a required data point from the file.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::vector<std::string> getClientRequiredDataPoints(std::string file_name) {
    std::vector<std::string> required_data;
    std::string root = createConfigPath(file_name);
    std::ifstream file(root);
    if (!file) {
        throw std::runtime_error("Invalid required Data Points file: " + file_name);
    }
    std::string line;
    while (std::getline(file, line)) {
        required_data.push_back(line);
    }
    return required_data;
}

/**
 * @brief Loads the model configuration from a JSON file.
 *
 * This function reads the model configuration from the specified JSON file and populates
 * the provided ModelConfig object with the configuration data. It validates the structure
 * of the JSON file and ensures that all required fields are present.
 *
 * @param config_file The path to the JSON configuration file.
 * @param model_config The ModelConfig object to be populated with the configuration data.
 *
 * @throws std::runtime_error If the configuration file cannot be opened or if required fields are
 * missing.
 */
void loadModelConfig(const std::string& config_file, ModelConfig& model_config) {
    std::ifstream file(config_file);
    if (!file) {
        throw std::runtime_error("Could not open the model config file: " + config_file);
    }

    json config_json;
    file >> config_json;

    // Validate the structure of the JSON
    validateJsonFields(config_json);

    if (config_json["reasoner_settings"]["supported_tree_types"].size() > 0) {
        const std::string generic_error_message =
            "Error in the model_config.json file. Missing required field";

        for (const auto& tree_type : config_json["reasoner_settings"]["supported_tree_types"]) {
            std::string lc_tree_type = Helper::toLowerCase(tree_type.get<std::string>());

            // Read supported tree types
            model_config.reasoner_settings.supported_tree_types.push_back(
                Helper::toUppercase(tree_type.get<std::string>()));

            // Read system data points for a tree type
            if (!config_json["inputs"].contains(lc_tree_type + "_data")) {
                throw std::runtime_error(generic_error_message + " in 'inputs': '" + lc_tree_type +
                                         "_data'");
            }
            model_config.system_data_points[lc_tree_type] = getClientRequiredDataPoints(
                config_json["inputs"][lc_tree_type + "_data"].get<std::string>());

            // Read tree type specific triple assembler helpers
            if (config_json["queries"]["triple_assembler_helper"].contains(lc_tree_type)) {
                std::vector<std::string> query_list;
                for (const auto& query :
                     config_json["queries"]["triple_assembler_helper"][lc_tree_type]) {
                    query_list.push_back(createConfigPath(query.get<std::string>()));
                }
                model_config.triple_assembler_queries_files[lc_tree_type] = query_list;
            } else {
                std::cout
                    << (" ** INFO: There is any triple assembler helper for the tree type: '" +
                        lc_tree_type + "' configured in the model_config.json")
                    << std::endl
                    << std::endl;
            }
        }
    } else {
        throw std::runtime_error("You need to add some supported tree types in " + config_file);
    }

    // Read output file path
    model_config.output_file_path = createConfigPath(config_json["output"].get<std::string>());

    // Read ontologies files
    for (const auto& ontology : config_json["ontologies"]) {
        model_config.ontology_files.push_back(createConfigPath(ontology.get<std::string>()));
    }

    // Read SHACL shapes files
    for (const auto& shacl : config_json["shacl"]) {
        model_config.shacl_shapes_files.push_back(createConfigPath(shacl.get<std::string>()));
    }

    // Read default triple assembler helpers
    std::vector<std::string> query_list;
    for (const auto& query : config_json["queries"]["triple_assembler_helper"]["default"]) {
        query_list.push_back(createConfigPath(query.get<std::string>()));
    }
    model_config.triple_assembler_queries_files["default"] = query_list;

    // Read rules paths
    for (const auto& rule : config_json["rules"]) {
        model_config.rules_files.push_back(createConfigPath(rule.get<std::string>()));
    }

    // Read output queries path
    model_config.output_queries_path =
        createConfigPath(config_json["queries"]["output"].get<std::string>());

    // Read reasoner settings
    model_config.reasoner_settings.inference_engine =
        config_json["reasoner_settings"]["inference_engine"].get<std::string>();
    model_config.reasoner_settings.output_format = reasonerOutputFormatToRDFSyntaxType(
        config_json["reasoner_settings"]["output_format"].get<std::string>());
}
}  // namespace ModelConfigUtils