#include "reasoning_query_service.h"

#include "data_types.h"
#include "json_writer.h"

ReasoningQueryService::ReasoningQueryService(std::shared_ptr<ReasonerService> reasoning_service)
    : reasoning_service_(reasoning_service) {}

/**
 * Processes a reasoning query and returns the result in JSON format.
 *
 * @param reasoning_output_query The reasoning output query containing the query
 * string, query language, and other relevant details.
 * @param is_ai_reasoner_inference_results A boolean indicating whether the reasoning results are
 * inferred.
 * @param output_file_path An optional string representing the path to an output file where results
 * may be saved.
 * @return A nlohmann::json object containing the results of the reasoning query.
 */
nlohmann::json ReasoningQueryService::processReasoningQuery(
    const ReasoningOutputQuery& reasoning_output_query, const bool is_ai_reasoner_inference_results,
    const std::optional<std::string>& output_file_path) {
    nlohmann::json result;

    // Process each query
    std::string query_result = reasoning_service_->queryData(reasoning_output_query.query,
                                                             reasoning_output_query.query_language,
                                                             DataQueryAcceptType::SPARQL_JSON);

    return JSONWriter::writeToJson(query_result, DataQueryAcceptType::SPARQL_JSON,
                                   is_ai_reasoner_inference_results, output_file_path);
}
