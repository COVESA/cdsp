#include "rdfox_adapter.h"

#include <iostream>

RDFoxAdapter::RDFoxAdapter(const ReasonerServerData& server_data)
    : host_(server_data.host),
      port_(server_data.port),
      auth_header_base64_("Basic " + server_data.auth_base64) {
    if (server_data.data_store_name.has_value()) {
        data_store_ = server_data.data_store_name.value();
    } else {
        throw std::runtime_error("Data store name must be provided.");
    }
}

std::unique_ptr<RequestBuilder> RDFoxAdapter::createRequestBuilder() const {
    return std::make_unique<RequestBuilder>(host_, port_, auth_header_base64_);
}

/**
 * @brief Initializes the RDFoxAdapter by ensuring the data store is created.
 *
 * This method checks if the data store specified by `data_store_` exists.
 * If the data store exists, it logs a message indicating its existence.
 * If the data store does not exist, it attempts to create it by sending
 * a POST request to the appropriate endpoint. If the creation is successful,
 * a success message is logged. Otherwise, an exception is thrown.
 *
 * @throws std::runtime_error if the data store creation fails.
 */
void RDFoxAdapter::initialize() {
    std::cout << "** Initializing RDFox adapter **" << std::endl;

    std::cout << " - Starting data store: " << data_store_ << std::endl;
    // checks if the data store exists, create it if not
    if (checkDataStore()) {
        std::cout << " - Data store '" + data_store_ + "' is already created." << std::endl;
    } else {
        std::cout << " - Data store '" << data_store_ << "' does not exist. Creating it..."
                  << std::endl;
        // Creates a data store
        std::string target = "/datastores/" + data_store_;

        if (createRequestBuilder()
                ->setMethod(http::verb::post)
                .setTarget(target)
                .setContentType("application/json")
                .sendRequest()) {
            std::cout << " - Data store '" << data_store_ << "' created successfully." << std::endl;
        } else {
            throw std::runtime_error("Failed to create datastore '" + data_store_ + "'");
        }
    }
}

/**
 * @brief Checks if the data store exists on the server.
 *
 * @return true if the data store is found in the server's response; false otherwise.
 */
bool RDFoxAdapter::checkDataStore() {
    const std::string target = "/datastores";
    std::string response_body;
    if (createRequestBuilder()
            ->setMethod(http::verb::get)
            .setTarget(target)
            .setAcceptType("text/csv; charset=UTF-8")
            .sendRequest(nullptr, &response_body)) {
        return response_body.find(data_store_) != std::string::npos;
    }
    return false;
}

/**
 * Loads data into the RDFox datastore.
 *
 * This method sends a POST request to load data into the RDFox datastore.
 *
 * @param data The data to be loaded into the datastore.
 * @param content_type The content type of the data to be loaded. Default is "text/turtle".
 * @return true if the data is successfully loaded; false otherwise.
 */
bool RDFoxAdapter::loadData(const std::string& data, const std::string& content_type) {
    std::string target = "/datastores/" + data_store_ + "/content";

    return createRequestBuilder()
        ->setMethod(http::verb::post)
        .setTarget(target)
        .setContentType(content_type)
        .setBody(data)
        .sendRequest();
};

/**
 * Queries data from the RDFox datastore.
 *
 * This method sends a POST request to query data from the RDFox datastore.
 *
 * @param sparql_query The SPARQL query to be executed.
 * @param query_language_type The query language type. Default is "SPARQL".
 * @param accept_type The accept type of the response. Default is "table/csv".
 * @return The response body of the query.
 */
std::string RDFoxAdapter::queryData(const std::string& sparql_query,
                                    const QueryLanguageType& query_language_type,
                                    const DataQueryAcceptType& accept_type) {
    std::string target = "/datastores/" + data_store_ + "/sparql";
    std::string response_body;
    return createRequestBuilder()
                   ->setMethod(http::verb::post)
                   .setTarget(target)
                   .setContentType(queryLanguageTypeToContentType(query_language_type))
                   .setBody(sparql_query)
                   .setAcceptType(queryAcceptTypeToString(accept_type))
                   .sendRequest(nullptr, &response_body)
               ? response_body
               : "";
}

/**
 * Deletes the current RDFox datastore if it exists.
 *
 * This method checks if the datastore is present and attempts to delete it.
 * It logs the outcome of the operation to the standard output.
 *
 * @return A boolean value indicating the success of the operation.
 *         - Returns `true` if the datastore does not exist or if it was successfully deleted.
 *         - Returns `false` if the datastore exists but could not be deleted.
 */
bool RDFoxAdapter::deleteDataStore() {
    if (checkDataStore()) {
        std::string target = "/datastores/" + data_store_;

        if (createRequestBuilder()
                ->setMethod(http::verb::delete_)
                .setTarget(target)
                .sendRequest()) {
            std::cout << " - Data store '" + data_store_ + "' have been removed successfully."
                      << std::endl;
        } else {
            std::cout << " - Data store '" + data_store_ + "' could not be removed." << std::endl;
            return false;
        }
    } else {
        std::cout << " - Data store '" + data_store_ + "' does not exists anymore." << std::endl;
    }
    return true;
}

/**
 * Creates a connection to the RDFox datastore.
 *
 * This method sends a POST request to create a connection to the RDFox datastore.
 * If the connection is successfully created, it extracts and returns the connection ID
 * and authentication token from the response response_headers.
 *
 * @return A pair containing the connection ID and authentication token.
 *         - The first element is the connection ID.
 *         - The second element is the authentication token.
 * @throws std::runtime_error if the connection could not be created.
 */
std::pair<std::string, std::string> RDFoxAdapter::createConnection() {
    std::string target = "/datastores/" + data_store_ + "/connections";
    auto response_headers = std::map<std::string, std::string>();

    // Send a POST request to create a connection
    if (!createRequestBuilder()
             ->setMethod(http::verb::post)
             .setTarget(target)
             .setContentType("application/json")
             .sendRequest(&response_headers)) {
        throw std::runtime_error("Failed request creating a connection in RDFox.");
    }

    auto location = response_headers.find("Location");
    auto token = response_headers.find("RDFox-Authentication-Token");

    if (location == response_headers.end()) {
        throw std::runtime_error(
            "Missing 'Location' header in the response creating a connection with RDFox.");
    }

    if (token == response_headers.end()) {
        throw std::runtime_error(
            "Missing 'RDFox-Authentication-Token' header in the response creating a connection "
            "with RDFox");
    }

    // Extract connection ID from the "Location" header
    const std::string& location_path = location->second;
    std::size_t last_slash = location_path.find_last_of('/');
    if (last_slash == std::string::npos) {
        throw std::runtime_error(
            "Invalid 'Location' header format creating a connection with RDFox: " + location_path);
    }

    const std::string connection_id = location_path.substr(last_slash + 1);
    const std::string authentication_token = "RDFox " + token->second;

    return std::make_pair(connection_id, authentication_token);
}

bool RDFoxAdapter::checkConnection(const std::string& connection_id) {
    std::string target = "/datastores/" + data_store_ + "/connections/" + connection_id;

    return createRequestBuilder()->setMethod(http::verb::get).setTarget(target).sendRequest();
}

/**
 * Creates a cursor for a specified connection within the RDFox datastore.
 *
 * This method sends a POST request to create a cursor associated with the given connection ID.
 * The cursor is created by sending a SPARQL query to the RDFox datastore.
 *
 * @param connection_id The ID of the connection for which the cursor is to be created.
 * @param auth_token The authentication token for the connection.
 * @param query The SPARQL query to be executed to create the cursor.
 * @return The ID of the created cursor.
 * @throws std::runtime_error if the cursor could not be created.
 */
std::string RDFoxAdapter::createCursor(const std::string& connection_id,
                                       const std::string& auth_token, const std::string& query) {
    std::string target =
        "/datastores/" + data_store_ + "/connections/" + connection_id + "/cursors";
    auto response_headers = std::map<std::string, std::string>();

    // Send a POST request to create a cursor
    if (!createRequestBuilder()
             ->setMethod(http::verb::post)
             .setTarget(target)
             .setContentType("application/sparql-query")
             .setAuthorization(auth_token)
             .setBody(query)
             .sendRequest(&response_headers)) {
        throw std::runtime_error("Failed to create a new cursor for the connection in RDFox.");
    }

    auto location = response_headers.find("Location");
    if (location == response_headers.end()) {
        throw std::runtime_error(
            "Missing 'Location' header in the response creating a cursor in RDFox.");
    }

    // Extract cursor ID from the "Location" header
    const std::string& location_path = location->second;
    std::size_t last_slash = location_path.find_last_of('/');
    if (last_slash == std::string::npos) {
        throw std::runtime_error("Invalid 'Location' header format creating a cursor with RDFox: " +
                                 location_path);
    }
    return location_path.substr(last_slash + 1);
}

/**
 * Advances or opens a cursor for a specified connection within the RDFox datastore.
 *
 * This method sends an HTTP PATCH request to either open or advance a cursor associated with the
 * given connection and cursor IDs. The operation must be either "open" or "advance". Optionally, a
 * limit can be specified to restrict the number of results.
 *
 * @param connection_id The ID of the connection for which the cursor operation is to be performed.
 * @param cursor_id The ID of the cursor to be opened or advanced.
 * @param operation A string specifying the operation to perform on the cursor. Valid values are
 * "open" and "advance".
 * @param response A pointer to a string where the response body will be stored.
 * @param limit An optional integer specifying the maximum number of results to return. If not
 * provided, no limit is applied. Parameter `limit=0` can be used to specify that no answers should
 * be returned (and so the request just validates the cursor) .
 * @return A boolean value indicating the success of the operation.
 *         - Returns `true` if the request was successful.
 *         - Returns `false` if the operation is invalid or if the request fails.
 */
bool RDFoxAdapter::advanceCursor(const std::string& connection_id, const std::string& auth_token,
                                 const std::string& cursor_id,
                                 const DataQueryAcceptType& accept_type,
                                 const std::string& operation, std::optional<int> limit,
                                 std::string* response) {
    if (operation != "open" && operation != "advance") {
        std::cerr << "Invalid operation: " << operation << std::endl;
        return false;
    }

    std::string target = "/datastores/" + data_store_ + "/connections/" + connection_id +
                         "/cursors/" + cursor_id + "?operation=" + operation;
    if (limit.has_value()) {
        target += "&limit=" + std::to_string(limit.value());
    }

    return createRequestBuilder()
        ->setMethod(http::verb::patch)
        .setAuthorization(auth_token)
        .setTarget(target)
        .setAcceptType(queryAcceptTypeToString(accept_type))
        .sendRequest(nullptr, response);
}

/**
 * Deletes a cursor associated with a specified connection within the RDFox datastore.
 *
 * This method removes a cursor identified by the given connection ID
 * and cursor ID. It constructs the target URL using these identifiers and attempts to perform the
 * deletion.
 *
 * @param connection_id The ID of the connection from which the cursor is to be deleted.
 * @param cursor_id The ID of the cursor to be deleted.
 * @return A boolean value indicating the success of the deletion operation.
 *         - Returns `true` if the cursor was successfully deleted.
 *         - Returns `false` if the deletion request fails.
 */
bool RDFoxAdapter::deleteCursor(const std::string& connection_id, const std::string& cursor_id) {
    std::string target =
        "/datastores/" + data_store_ + "/connections/" + connection_id + "/cursors/" + cursor_id;

    return createRequestBuilder()->setMethod(http::verb::delete_).setTarget(target).sendRequest();
}