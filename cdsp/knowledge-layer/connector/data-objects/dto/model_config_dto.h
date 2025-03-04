#ifndef MODEL_CONFIG_DTO_H
#define MODEL_CONFIG_DTO_H

#include <map>
#include <string>
#include <vector>

/**
 * @brief Data Transfer Object for the Triple Assembler Helper
 */
struct TripleAssemblerHelperDTO {
    std::map<std::string, std::vector<std::string>> queries;
    std::string output;

    TripleAssemblerHelperDTO() : output("") {}

    // Overload the << operator to print the TripleAssemblerHelperDTO
    friend std::ostream& operator<<(std::ostream& os, const TripleAssemblerHelperDTO& dto) {
        os << "    TripleAssemblerHelperDTO {\n";
        os << "      queries: \n";
        os << "      {\n";
        for (const auto& [key, value] : dto.queries) {
            os << "        " << key << ": [\n";
            for (const auto& query : value) {
                os << "          " << query << ",\n";
            }
            os << "        ]\n";
        }
        os << "      }\n";
        os << "      output: " << dto.output << "\n";
        os << "    }";
        return os;
    }
};

/**
 * @brief Data Transfer Object for the Reasoner Settings
 */
struct ReasonerSettingsDTO {
    std::string inference_engine;
    std::string output_format;
    std::vector<std::string> supported_schema_collections;

    // Overload the << operator to print the ReasonerSettingsDTO
    friend std::ostream& operator<<(std::ostream& os, const ReasonerSettingsDTO& dto) {
        os << "    ReasonerSettingsDTO {\n"
           << "      inference_engine: " << dto.inference_engine << "\n"
           << "      output_format: " << dto.output_format << "\n"
           << "      supported_schema_collections: [\n";
        for (const auto& schema : dto.supported_schema_collections) {
            os << "        " << schema << ",\n";
        }
        os << "      ]\n";
        os << "    }";
        return os;
    }
};

/**
 * @brief Data Transfer Object for the Model Configuration
 */
struct ModelConfigDTO {
    std::map<std::string, std::string> inputs;
    std::vector<std::string> ontologies;
    std::string output;
    TripleAssemblerHelperDTO queries_config;
    std::vector<std::string> rules;
    std::vector<std::string> shacl_shapes;
    ReasonerSettingsDTO reasoner_settings;

    ModelConfigDTO()
        : inputs(),
          ontologies(),
          output(""),
          queries_config(),
          rules(),
          shacl_shapes(),
          reasoner_settings() {}

    // Overload the << operator to print the ModelConfigDTO
    friend std::ostream& operator<<(std::ostream& os, const ModelConfigDTO& dto) {
        os << "ModelConfigDTO {\n";
        os << "  inputs: {\n";
        for (const auto& [key, value] : dto.inputs) {
            os << "    " << key << ": " << value << "\n";
        }
        os << "  }\n";
        os << "  ontologies: [\n";
        for (const auto& ontology : dto.ontologies) {
            os << "    " << ontology << ",\n";
        }
        os << "  ]\n";
        os << "  output: " << dto.output << "\n";
        os << "  queries_config:\n" << dto.queries_config << "\n";
        os << "  rules: [\n";
        for (const auto& rule : dto.rules) {
            os << "    " << rule << ",\n";
        }
        os << "  ]\n";
        os << "  shacl_shapes: [\n";
        for (const auto& shacl_shape : dto.shacl_shapes) {
            os << "    " << shacl_shape << ",\n";
        }
        os << "  ]\n";
        os << "  reasoner_settings:\n" << dto.reasoner_settings << "\n";
        os << "}";
        return os;
    }
};

#endif  // MODEL_CONFIG_DTO_H