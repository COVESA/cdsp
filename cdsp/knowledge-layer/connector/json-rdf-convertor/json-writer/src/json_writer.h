#pragma once

#include <optional>
#include <string>

#include "data_types.h"
#include "i_file_handler.h"

class JSONWriter {
   public:
    static nlohmann::json writeToJson(const std::string &query_result,
                                      const DataQueryAcceptType &result_format_type,
                                      bool is_ai_reasoner_inference_results = false,
                                      std::optional<std::string> output_file_path = std::nullopt,
                                      const std::shared_ptr<IFileHandler> &file_handler = nullptr);

   private:
    static nlohmann::json parseQueryResult(const std::string &query_result,
                                           const DataQueryAcceptType &result_format_type);
    static nlohmann::json groupResult(const nlohmann::json &flat_result,
                                      bool is_ai_reasoner_inference_results);
    static nlohmann::json groupItem(const nlohmann::json &item,
                                    bool is_ai_reasoner_inference_results);
    static void handleAIReasonerInferenceResults(nlohmann::json &grouped);
    static nlohmann::json parseTableFormat(const std::string &query_result, char delimiter);
    static nlohmann::json parseSparqlJson(const std::string &json_result);
    static nlohmann::json parseSparqlXml(const std::string &xml_result);

    static void storeJsonToFile(const nlohmann::json &json_data,
                                const std::string &output_file_path,
                                const std::shared_ptr<IFileHandler> &file_handler);
};