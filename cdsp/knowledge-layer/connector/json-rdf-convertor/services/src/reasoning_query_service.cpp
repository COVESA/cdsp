#include "reasoning_query_service.h"

#include <iostream>

#include "helper.h"

ReasoningQueryService::ReasoningQueryService(std::shared_ptr<ReasonerService> reasoning_service)
    : reasoning_service_(reasoning_service) {}

/**
 * Processes a reasoning query and returns the result in JSON format.
 *
 * @param reasoning_output_query A pair consisting of the query language type and the query string.
 *                               The first element specifies the type of query language (e.g.,
 * SPARQL), and the second element contains the actual query to be executed.
 * @return A nlohmann::json object containing the result of the executed query.
 */
nlohmann::json ReasoningQueryService::processReasoningQuery(
    const std::pair<QueryLanguageType, std::string>& reasoning_output_query,
    const std::optional<std::string>& output_file_path) {
    nlohmann::json result;

    // Process each query
    std::string query_result =
        reasoning_service_->queryData(reasoning_output_query.second, reasoning_output_query.first,
                                      DataQueryAcceptType::SPARQL_JSON);
    return JSONWriter::writeToJson(query_result, DataQueryAcceptType::SPARQL_JSON,
                                   output_file_path);
}
