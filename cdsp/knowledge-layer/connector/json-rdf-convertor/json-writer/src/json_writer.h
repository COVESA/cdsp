#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <optional>
#include <string>

#include "data_types.h"
#include "i_file_handler.h"
#include "nlohmann/json.hpp"

class JSONWriter {
   public:
    static nlohmann::json writeToJson(const std::string& query_result,
                                      const DataQueryAcceptType& result_format_type,
                                      const bool is_ai_reasoner_inference_results = false,
                                      std::optional<std::string> output_file_path = std::nullopt,
                                      std::shared_ptr<IFileHandler> file_handler = nullptr);

   private:
    static nlohmann::json parseTableFormat(const std::string& query_result, char delimiter);
    static nlohmann::json parseSparqlJson(const std::string& json_result);
    static nlohmann::json parseSparqlXml(const std::string& xml_result);

    static void storeJsonToFile(const nlohmann::json& json_data,
                                const std::string& output_file_path,
                                std::shared_ptr<IFileHandler> file_handler);
};

#endif  // JSON_WRITER_H