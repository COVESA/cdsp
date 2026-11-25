#include "rdf4j_adapter.h"

#include <iostream>
#include <stdexcept>

#include "helper.h"

/**
 * @brief Creates the adapter for the RDF4J reasoner service.
 *
 * @param server_data Host, port, authorization and datastore name of the service.
 * @throws std::runtime_error if no datastore name is given.
 */
RDF4JAdapter::RDF4JAdapter(const ReasonerServerData& server_data)
    : host_(server_data.host),
      port_(server_data.port),
      auth_header_base64_("Basic " + server_data.auth_base64) {
    if (server_data.data_store_name.has_value()) {
        data_store_ = server_data.data_store_name.value();
    } else {
        throw std::runtime_error("Data store name must be provided.");
    }
}

/**
 * @brief Creates a request builder for the service. Tests replace it with a mock.
 *
 * @return A new request builder.
 */
std::unique_ptr<RequestBuilder> RDF4JAdapter::createRequestBuilder() const {
    return std::make_unique<RequestBuilder>(host_, port_, auth_header_base64_);
}

/**
 * @brief Creates the datastore if the service does not list it yet.
 *
 * @throws std::runtime_error if the datastore cannot be created.
 */
void RDF4JAdapter::initialize() {
    std::cout << "** Initializing RDF4J reasoner adapter **" << std::endl;
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

/**
 * @brief Checks whether the service lists the datastore, by its exact name.
 *
 * @return true if the datastore exists; false if not or if the request fails.
 */
bool RDF4JAdapter::checkDataStore() {
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

/**
 * @brief Loads data into the datastore.
 *
 * @param data The data, for example in Turtle.
 * @param content_type The content type of the data.
 * @return true if the data was loaded.
 */
bool RDF4JAdapter::loadData(const std::string& data, const std::string& content_type) {
    const std::string target = "/datastores/" + data_store_ + "/content";

    return createRequestBuilder()
        ->setMethod(http::verb::post)
        .setTarget(target)
        .setContentType(content_type)
        .setBody(data)
        .sendRequest();
}

/**
 * @brief Adds rules to the datastore.
 *
 * @param rules The rules, for example SHACL rules in Turtle.
 * @param content_type The content type of the rules.
 * @return true if the rules were added.
 */
bool RDF4JAdapter::loadRules(const std::string& rules, const std::string& content_type) {
    const std::string target = "/datastores/" + data_store_ + "/rules/add";

    return createRequestBuilder()
        ->setMethod(http::verb::post)
        .setTarget(target)
        .setContentType(content_type)
        .setBody(rules)
        .sendRequest();
}

/**
 * @brief Runs a query on the datastore.
 *
 * @param query The query text.
 * @param query_language_type The language of the query.
 * @param accept_type The format of the result.
 * @return The query result, or an empty string on error.
 */
std::string RDF4JAdapter::queryData(const std::string& query,
                                    const QueryLanguageType& query_language_type,
                                    const DataQueryAcceptType& accept_type) {
    const std::string target = "/datastores/" + data_store_ + "/sparql";
    std::string response_body;

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

/**
 * @brief Deletes the datastore if the service lists it.
 *
 * @return true if the datastore was deleted or did not exist; false if the deletion failed.
 */
bool RDF4JAdapter::deleteDataStore() {
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
