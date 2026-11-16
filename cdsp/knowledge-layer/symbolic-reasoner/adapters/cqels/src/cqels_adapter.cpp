#include "cqels_adapter.h"

#include <iostream>
#include <regex>
#include <stdexcept>

#include "helper.h"

CQELSAdapter::CQELSAdapter(const ReasonerServerData& server_data)
    : host_(server_data.host),
      port_(server_data.port),
      auth_header_base64_("Basic " + server_data.auth_base64) {
    if (server_data.data_store_name.has_value()) {
        data_store_ = server_data.data_store_name.value();
    } else {
        throw std::runtime_error("Data store name must be provided.");
    }
}

std::unique_ptr<RequestBuilder> CQELSAdapter::createRequestBuilder() const {
    return std::make_unique<RequestBuilder>(host_, port_, auth_header_base64_);
}

void CQELSAdapter::initialize() {
    std::cout << "** Initializing CQELS reasoner adapter **" << std::endl;
    std::cout << " - Starting data store: " << data_store_ << std::endl;

    if (checkDataStore()) {
        std::cout << " - Data store '" + data_store_ + "' is already created." << std::endl;
        return;
    }

    std::cout << " - Data store '" << data_store_ << "' does not exist. Creating it..."
              << std::endl;
    const std::string target = "/datastores/" + data_store_;

    if (createRequestBuilder()->setMethod(http::verb::post).setTarget(target).sendRequest()) {
        std::cout << " - Data store '" << data_store_ << "' created successfully." << std::endl;
        return;
    }

    throw std::runtime_error("Failed to create datastore '" + data_store_ + "'");
}

bool CQELSAdapter::checkDataStore() {
    const std::string target = "/datastores";
    std::string response_body;

    if (!createRequestBuilder()
             ->setMethod(http::verb::get)
             .setTarget(target)
             .setAcceptType("text/plain")
             .sendRequest(nullptr, &response_body)) {
        return false;
    }

    return Helper::containsListItem(response_body, data_store_);
}

bool CQELSAdapter::loadData(const std::string& data, const std::string& content_type) {
    const std::string target = "/datastores/" + data_store_ + "/content";

    return createRequestBuilder()
        ->setMethod(http::verb::post)
        .setTarget(target)
        .setContentType(content_type)
        .setBody(data)
        .sendRequest();
}

/**
 * @brief Pushes the observations as one element to every stream of the registered queries.
 *
 * @param data The observations.
 * @param content_type The content type of the observations.
 * @param observation_time The event time of the element.
 * @return true if the element was pushed to every stream.
 */
bool CQELSAdapter::loadObservations(const std::string& data, const std::string& content_type,
                                    std::chrono::system_clock::time_point observation_time) {
    if (streams_.empty()) {
        std::cerr << "No CQELS stream to push the observations to. Register a CQELS-QL query "
                     "with FROM STREAM first."
                  << std::endl;
        return false;
    }

    const std::string event_time =
        Helper::getFormattedTimestampCustom("%Y-%m-%dT%H:%M:%S", observation_time, true);
    bool pushed = true;

    for (const auto& stream : streams_) {
        const std::string target =
            "/datastores/" + data_store_ + "/streams/" + stream + "?eventTime=" + event_time;

        pushed = createRequestBuilder()
                     ->setMethod(http::verb::post)
                     .setTarget(target)
                     .setContentType(content_type)
                     .setBody(data)
                     .sendRequest() &&
                 pushed;
    }
    return pushed;
}

/**
 * @brief Registers a continuous CQELS-QL query and remembers its id and streams.
 *
 * @param rules The CQELS-QL query.
 * @param content_type The content type of the rules, must be the one of CQELS-QL.
 * @return true if the query was registered.
 */
bool CQELSAdapter::loadRules(const std::string& rules, const std::string& content_type) {
    if (content_type != ruleLanguageTypeToContentType(RuleLanguageType::CQELS)) {
        std::cerr << "The CQELS reasoner only accepts CQELS-QL rules (.cqels), not " << content_type
                  << std::endl;
        return false;
    }

    const std::string target = "/datastores/" + data_store_ + "/queries";
    std::string query_id;

    if (!createRequestBuilder()
             ->setMethod(http::verb::post)
             .setTarget(target)
             .setContentType(content_type)
             .setBody(rules)
             .sendRequest(nullptr, &query_id)) {
        return false;
    }

    query_ids_[rules] = query_id;

    const std::regex stream_regex(R"(FROM\s+STREAM\s+([\w-]+))", std::regex::icase);
    for (std::sregex_iterator it(rules.begin(), rules.end(), stream_regex), end; it != end; ++it) {
        streams_.insert((*it)[1]);
    }
    return true;
}

/**
 * @brief Returns the new results of a registered CQELS-QL query, or runs a SPARQL query on the
 * static data.
 *
 * @param query The query text. A CQELS-QL query must be registered with loadRules before.
 * @param query_language_type The language of the query.
 * @param accept_type The format of the result.
 * @return The query result, or an empty string on error.
 */
std::string CQELSAdapter::queryData(const std::string& query,
                                    const QueryLanguageType& query_language_type,
                                    const DataQueryAcceptType& accept_type) {
    std::string response_body;

    if (query_language_type != QueryLanguageType::CQELS) {
        const std::string target = "/datastores/" + data_store_ + "/sparql";

        return createRequestBuilder()
                       ->setMethod(http::verb::post)
                       .setTarget(target)
                       .setContentType(queryLanguageTypeToContentType(query_language_type))
                       .setBody(query)
                       .setAcceptType(queryAcceptTypeToString(accept_type))
                       .sendRequest(nullptr, &response_body)
                   ? response_body
                   : "";
    }

    const auto query_id = query_ids_.find(query);
    if (query_id == query_ids_.end()) {
        std::cerr << "CQELS-QL query is not registered. List it in the rules of the model "
                     "configuration too."
                  << std::endl;
        return "";
    }

    const std::string target =
        "/datastores/" + data_store_ + "/queries/" + query_id->second + "/results";

    return createRequestBuilder()
                   ->setMethod(http::verb::get)
                   .setTarget(target)
                   .setAcceptType(queryAcceptTypeToString(accept_type))
                   .sendRequest(nullptr, &response_body)
               ? response_body
               : "";
}

bool CQELSAdapter::deleteDataStore() {
    if (!checkDataStore()) {
        std::cout << " - Data store '" + data_store_ + "' does not exist." << std::endl;
        return true;
    }

    const std::string target = "/datastores/" + data_store_;

    if (createRequestBuilder()->setMethod(http::verb::delete_).setTarget(target).sendRequest()) {
        std::cout << " - Data store '" << data_store_ << "' deleted successfully." << std::endl;
        return true;
    }

    std::cout << " - Failed to delete data store '" << data_store_ << "'." << std::endl;
    return false;
}
