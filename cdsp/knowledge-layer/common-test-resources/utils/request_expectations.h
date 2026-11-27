#ifndef REQUEST_EXPECTATIONS_H
#define REQUEST_EXPECTATIONS_H

#include <gmock/gmock.h>

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "mock_request_builder.h"

/**
 * @brief One HTTP request an adapter is expected to send, and the answer it gets.
 *
 * Method and target are always checked. Every other field is checked when it is set, and must
 * not be called by the adapter when it is not set. The response body and headers are only
 * written when they are set, and the adapter must then ask for them.
 */
struct ExpectedRequest {
    ExpectedRequest(http::verb method, std::string target)
        : method(method), target(std::move(target)) {}

    ExpectedRequest& withContentType(const std::string& value) {
        content_type = value;
        return *this;
    }
    ExpectedRequest& withAcceptType(const std::string& value) {
        accept_type = value;
        return *this;
    }
    ExpectedRequest& withAuthorization(const std::string& value) {
        authorization = value;
        return *this;
    }
    ExpectedRequest& withBody(const std::string& value) {
        body = value;
        return *this;
    }
    ExpectedRequest& respondsWith(const std::string& value) {
        response_body = value;
        return *this;
    }
    ExpectedRequest& respondsWithHeaders(const std::map<std::string, std::string>& value) {
        response_headers = value;
        return *this;
    }
    ExpectedRequest& fails() {
        succeeds = false;
        return *this;
    }

    http::verb method;
    std::string target;
    std::optional<std::string> content_type;
    std::optional<std::string> accept_type;
    std::optional<std::string> authorization;
    std::optional<std::string> body;
    std::optional<std::string> response_body;
    std::optional<std::map<std::string, std::string>> response_headers;
    bool succeeds = true;
};

/**
 * @brief Expects the adapter to create one request builder per request, in the given order, and
 * to send each request exactly as described. Any other call on a builder fails the test.
 *
 * @param adapter A mock adapter with a mocked createRequestBuilder().
 * @param requests The requests in the order the adapter sends them. Empty means no request.
 */
template <typename MockAdapter>
void expectRequests(MockAdapter& adapter, const std::vector<ExpectedRequest>& requests) {
    using testing::ReturnRef;

    auto& create =
        EXPECT_CALL(adapter, createRequestBuilder()).Times(static_cast<int>(requests.size()));

    for (const auto& request : requests) {
        auto builder = std::make_unique<testing::StrictMock<MockRequestBuilder>>("", "", "");

        EXPECT_CALL(*builder, setMethod(request.method)).WillOnce(ReturnRef(*builder));
        EXPECT_CALL(*builder, setTarget(request.target)).WillOnce(ReturnRef(*builder));
        if (request.content_type) {
            EXPECT_CALL(*builder, setContentType(*request.content_type))
                .WillOnce(ReturnRef(*builder));
        }
        if (request.accept_type) {
            EXPECT_CALL(*builder, setAcceptType(*request.accept_type))
                .WillOnce(ReturnRef(*builder));
        }
        if (request.authorization) {
            EXPECT_CALL(*builder, setAuthorization(*request.authorization))
                .WillOnce(ReturnRef(*builder));
        }
        if (request.body) {
            EXPECT_CALL(*builder, setBody(*request.body)).WillOnce(ReturnRef(*builder));
        }

        const testing::Matcher<std::map<std::string, std::string>*> headers_matcher =
            request.response_headers
                ? testing::Matcher<std::map<std::string, std::string>*>(testing::NotNull())
                : testing::IsNull();
        const testing::Matcher<std::string*> body_matcher =
            request.response_body ? testing::Matcher<std::string*>(testing::NotNull())
                                  : testing::IsNull();

        EXPECT_CALL(*builder, sendRequest(headers_matcher, body_matcher))
            .WillOnce([request](std::map<std::string, std::string>* headers, std::string* body) {
                if (request.response_headers) {
                    *headers = *request.response_headers;
                }
                if (request.response_body) {
                    *body = *request.response_body;
                }
                return request.succeeds;
            });

        create.WillOnce(
            testing::Return(testing::ByMove(std::unique_ptr<RequestBuilder>(std::move(builder)))));
    }
}

#endif  // REQUEST_EXPECTATIONS_H
