#include "request_builder.h"

#include <iostream>

/**
 * @brief Creates a builder for one HTTP request to the reasoner server.
 *
 * @param host The host of the reasoner server.
 * @param port The port of the reasoner server.
 * @param auth_base64 The value of the Authorization header.
 */
RequestBuilder::RequestBuilder(const std::string& host, const std::string& port,
                               const std::string& auth_base64)
    : host_(host), port_(port), auth_header_base64_(auth_base64) {}

/**
 * @brief Sets the HTTP method. The default is GET.
 *
 * @param method The HTTP method.
 * @return The builder, to chain further calls.
 */
RequestBuilder& RequestBuilder::setMethod(http::verb method) {
    method_ = method;
    return *this;
}

/**
 * @brief Sets the request target: the path, with an optional query string.
 *
 * @param target The request target, for example "/datastores".
 * @return The builder, to chain further calls.
 */
RequestBuilder& RequestBuilder::setTarget(const std::string& target) {
    target_ = target;
    return *this;
}

/**
 * @brief Replaces the Authorization header given in the constructor.
 *
 * @param auth_header_base64 The value of the Authorization header.
 * @return The builder, to chain further calls.
 */
RequestBuilder& RequestBuilder::setAuthorization(const std::string& auth_header_base64) {
    auth_header_base64_ = auth_header_base64;
    return *this;
}

/**
 * @brief Sets the Content-Type header. Not sent when empty.
 *
 * @param content_type The media type of the body.
 * @return The builder, to chain further calls.
 */
RequestBuilder& RequestBuilder::setContentType(const std::string& content_type) {
    content_type_ = content_type;
    return *this;
}

/**
 * @brief Sets the Accept header. Not sent when empty.
 *
 * @param accept_type The media type expected in the response.
 * @return The builder, to chain further calls.
 */
RequestBuilder& RequestBuilder::setAcceptType(const std::string& accept_type) {
    accept_type_ = accept_type;
    return *this;
}

/**
 * @brief Sets the request body.
 *
 * @param body The request body.
 * @return The builder, to chain further calls.
 */
RequestBuilder& RequestBuilder::setBody(const std::string& body) {
    body_ = body;
    return *this;
}

/**
 * @brief Sends the request and waits for the response.
 *
 * Host, port and target must be set. The response body and headers are written when the
 * pointers are given, also for an error status. Errors are logged, never thrown.
 *
 * @param headers Receives the response headers, if not null.
 * @param response_body Receives the response body, if not null.
 * @return true if the server answered with 200, 201 or 204; false on any other status or on a
 * network error.
 */
bool RequestBuilder::sendRequest(std::map<std::string, std::string>* headers,
                                 std::string* response_body) {
    net::io_context ioc;
    tcp::resolver resolver(ioc);
    tcp::socket socket(ioc);

    try {
        if (!validateRequiredFields()) {
            throw std::runtime_error("Required request fields are not set.");
        }
        // Resolve and connect to the host
        auto const results = resolver.resolve(host_, port_);
        net::connect(socket, results.begin(), results.end());

        // Set up the HTTP request
        http::request<http::string_body> req{method_, target_, 11};
        req.set(http::field::host, host_);
        req.set(http::field::authorization, auth_header_base64_);
        if (!content_type_.empty()) {
            req.set(http::field::content_type, content_type_);
        }
        if (!accept_type_.empty()) {
            req.set(http::field::accept, accept_type_);
        }
        req.body() = body_;
        req.prepare_payload();

        // Send the request and receive the response
        http::write(socket, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(socket, buffer, res);

        // Capture the response body
        if (response_body != nullptr) {
            *response_body = res.body();
        }

        // Extract response headers for verbose output
        if (headers != nullptr) {
            for (const auto& header : res.base()) {
                headers->emplace(std::string(header.name_string()), std::string(header.value()));
            }
        }

        // Check response status
        if (res.result() != http::status::ok && res.result() != http::status::created &&
            res.result() != http::status::no_content) {
            std::cerr << createErrorMessage(res.body(), res.result_int()) << std::endl;
            return false;
        }

        return true;
    } catch (const beast::system_error& e) {
        std::cerr << "Network error with reasoner server " << host_ << ":" << port_
                  << " (check HOST_REASONER_SERVER and PORT_REASONER_SERVER): " << e.what()
                  << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error in request: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief Builds the log message for an error status.
 *
 * @param error_msg The response body.
 * @param error_code The HTTP status code.
 * @return The message, for example "HTTP error 400: ...".
 */
std::string RequestBuilder::createErrorMessage(const std::string& error_msg, int error_code) {
    return "HTTP error " + std::to_string(error_code) + ": " + error_msg;
}

/**
 * @brief Checks that host, port and target are set.
 *
 * @return true if the request can be sent.
 */
bool RequestBuilder::validateRequiredFields() {
    return !host_.empty() && !port_.empty() && !target_.empty();
}