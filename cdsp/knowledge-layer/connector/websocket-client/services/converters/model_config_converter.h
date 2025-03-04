#ifndef MODEL_CONFIG_CONVERTER_H
#define MODEL_CONFIG_CONVERTER_H

#include "i_file_handler.h"
#include "model_config.h"
#include "model_config_dto.h"

class ModelConfigConverter {
   public:
    static const std::string
        INPUT_SUFFIX;  // Suffix that will be removed from the input collection name

    explicit ModelConfigConverter(std::shared_ptr<IFileHandler> file_handler);
    ModelConfig convert(const ModelConfigDTO& dto);

   private:
    const std::string PATH_TO_MODEL_FILES =
        std::string(PROJECT_ROOT) + "/symbolic-reasoner/examples/use-case/model/";
    std::shared_ptr<IFileHandler> file_handler_;

    ReasonerSettings convertReasonerSettings(const ReasonerSettingsDTO& dto);
    TripleAssemblerHelper convertTripleAssemblerHelper(const TripleAssemblerHelperDTO& dto);
    std::map<SchemaType, std::vector<std::string>> getInputsFromDto(
        const std::map<std::string, std::string>& inputs);
    std::vector<std::string> getSupportedDataPoints(std::string file_name);
    std::string getFullModelConfigPath(const std::string& model_config_file);
    std::vector<std::pair<ReasonerSyntaxType, std::string>> getReasonerSyntaxTypeAndContent(
        const std::vector<std::string>& file_list);
    ReasonerSyntaxType getReasonerSyntaxTypeFromFile(const std::string& file_path);

    TripleAssemblerHelper::QueryPair getQueriesToCreateTriples(
        const std::vector<std::string>& queries);
    std::vector<std::pair<RuleLanguageType, std::string>> getReasonerRules(
        const std::vector<std::string>& file_list);
};

#endif  // MODEL_CONFIG_CONVERTER_H