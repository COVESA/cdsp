#include "request_builder.h"

#include <iostream>

RequestBuilder::RequestBuilder(const std::string& host, const std::string& port,
                               const std::string& auth_base64)
    : host_(host), port_(port), auth_header_base64_(auth_base64) {}

RequestBuilder& RequestBuilder::setMethod(http::verb method) {
    method_ = method;
    return *this;
}

RequestBuilder& RequestBuilder::setTarget(const std::string& target) {
    target_ = target;
    return *this;
}

RequestBuilder& RequestBuilder::setAuthorization(const std::string& auth_header_base64) {
    auth_header_base64_ = auth_header_base64;
    return *this;
}

RequestBuilder& RequestBuilder::setContentType(const std::string& content_type) {
    content_type_ = content_type;
    return *this;
}

RequestBuilder& RequestBuilder::setAcceptType(const std::string& accept_type) {
    accept_type_ = accept_type;
    return *this;
}

RequestBuilder& RequestBuilder::setBody(const std::string& body) {
    body_ = body;
    return *this;
}

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
        std::cerr << "Network error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error in request: " << e.what() << std::endl;
        return false;
    }
}

std::string RequestBuilder::createErrorMessage(const std::string& error_msg, int error_code) {
    return "HTTP error " + std::to_string(error_code) + ": " + error_msg;
}

bool RequestBuilder::validateRequiredFields() {
    return !host_.empty() && !port_.empty() && !target_.empty();
}