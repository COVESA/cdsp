#include "rdfox_adapter.h"

#include <iostream>

RDFoxAdapter::RDFoxAdapter(const std::string& host, const std::string& port,
                           const std::string& auth_base64,
                           const std::string& data_store = "vehicle_ds")
    : host_(host),
      port_(port),
      auth_header_base64_("Basic " + auth_base64),
      data_store_(data_store) {}

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
    std::cout << "Initializing RDFox adapter ... " << std::endl;
    std::cout << " - Starting data store: " << data_store_ << std::endl;
    // checks if the data store exists, create it if not
    if (checkDataStore()) {
        std::cout << " - Data store '" + data_store_ + "' is already created." << std::endl;
    } else {
        std::cout << " - Data store '" << data_store_ << "' does not exist. Creating it..."
                  << std::endl;
        // Creates a data store
        std::string target = "/datastores/" + data_store_;
        if (sendPostRequest(target, "", "application/json")) {
            std::cout << " - Data store '" << data_store_ << "' created successfully." << std::endl;
        } else {
            throw std::runtime_error("Failed to create datastore '" + data_store_ + "'");
        }
    }
}

/**
 * Loads Turtle data into the RDFox datastore.
 *
 * @param ttl_data A string containing the Turtle data to be loaded into the datastore.
 * @return A boolean value indicating whether the data was successfully loaded.
 */
bool RDFoxAdapter::loadData(const std::string& ttl_data) {
    std::string target = "/datastores/" + data_store_ + "/content";
    return sendPostRequest(target, ttl_data, "text/turtle") ? true : false;
};

/**
 * Executes a SPARQL query against the RDFox datastore.
 *
 * @param sparql_query The SPARQL query string to be executed.
 * @return The response from the datastore as a string if the query is successful,
 *         otherwise an empty string.
 */
std::string RDFoxAdapter::queryData(const std::string& sparql_query) {
    std::string target = "/datastores/" + data_store_ + "/sparql";
    auto response = sendPostRequest(target, sparql_query, "application/sparql-query");
    return response.has_value() ? response.value() : "";
}

bool RDFoxAdapter::deleteDataStore() {
    if (checkDataStore()) {
        std::string target = "/datastores/" + data_store_;
        std::string responseBody;
        if (sendRequest(http::verb::delete_, target, "", "", "", responseBody)) {
            std::cout << "Data store '" + data_store_ + "' have been removed successfully."
                      << std::endl;
        } else {
            std::cout << "Data store '" + data_store_ + "' could not be removed." << std::endl;
            return false;
        }
    } else {
        std::cout << "Data store '" + data_store_ + "' does not exists anymore." << std::endl;
    }
    return true;
}

/**
 * @brief Checks if the data store exists on the server.
 *
 * @return true if the data store is found in the server's response; false otherwise.
 */
bool RDFoxAdapter::checkDataStore() {
    std::string target = "/datastores";
    std::string response = sendGetRequest(target, "text/csv; charset=UTF-8");
    if (response.find(data_store_) != std::string::npos) {
        return true;
    } else {
        return false;
    }
}

std::optional<std::string> RDFoxAdapter::sendPostRequest(const std::string& target,
                                                         const std::string& body,
                                                         const std::string& contentType) {
    std::string responseBody;
    if (sendRequest(http::verb::post, target, body, contentType, "", responseBody)) {
        return responseBody;
    }
    return std::nullopt;
}

std::string RDFoxAdapter::sendGetRequest(const std::string& target, const std::string& acceptType) {
    std::string responseBody;
    if (sendRequest(http::verb::get, target, "", "", acceptType, responseBody)) {
        return responseBody;
    }
    return "";
}

/**
 * Sends an HTTP request to a specified target and retrieves the response.
 *
 * @param method The HTTP method to use for the request (e.g., GET, POST).
 * @param target The target URI for the request.
 * @param body The body content to send with the request.
 * @param contentType The MIME type of the body content.
 * @param acceptType The MIME type that the client is willing to accept in the response.
 * @param responseBody A reference to a string where the response body will be stored.
 * @return True if the request was successful and the response status is OK, Created, or No Content;
 * false otherwise.
 */
bool RDFoxAdapter::sendRequest(http::verb method, const std::string& target,
                               const std::string& body, const std::string& contentType,
                               const std::string& acceptType, std::string& responseBody) {
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    tcp::socket socket(ioc);

    try {
        // Resolve and connect to the host
        auto const results = resolver.resolve(host_, port_);
        net::connect(socket, results.begin(), results.end());

        // Set up the HTTP request
        http::request<http::string_body> req{method, target, 11};
        req.set(http::field::host, host_);
        req.set(http::field::authorization, auth_header_base64_);
        if (!contentType.empty()) {
            req.set(http::field::content_type, contentType);
        }
        if (!acceptType.empty()) {
            req.set(http::field::accept, acceptType);
        }
        req.body() = body;
        req.prepare_payload();

        // Send the request and receive the response
        http::write(socket, req);
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(socket, buffer, res);

        responseBody = res.body();

        if (res.result() != http::status::ok && res.result() != http::status::created &&
            res.result() != http::status::no_content) {
            std::cerr << createErrorMessage(res.body(), res.result_int()) << std::endl;
            return false;
        }
        return true;
    } catch (const beast::system_error& e) {
        std::cerr << "Network error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error in request: " << e.what() << std::endl;
        return false;
    }
}

std::string RDFoxAdapter::createErrorMessage(const std::string& error_msg, int error_code) {
    return error_msg + " (Code: " + std::to_string(error_code) + ")";
}
