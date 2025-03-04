#ifndef MOCK_REQUEST_BUILDER_H
#define MOCK_REQUEST_BUILDER_H

#include <gmock/gmock.h>

#include "request_builder.h"

class MockRequestBuilder : public RequestBuilder {
   public:
    MockRequestBuilder(const std::string& host, const std::string& port, const std::string& auth)
        : RequestBuilder(host, port, auth) {}
    MOCK_METHOD(RequestBuilder&, setMethod, (http::verb method), (override));
    MOCK_METHOD(RequestBuilder&, setTarget, (const std::string& target), (override));
    MOCK_METHOD(RequestBuilder&, setAuthorization, (const std::string& auth_header_base64),
                (override));
    MOCK_METHOD(RequestBuilder&, setContentType, (const std::string& content_type), (override));
    MOCK_METHOD(RequestBuilder&, setAcceptType, (const std::string& accept_type), (override));
    MOCK_METHOD(RequestBuilder&, setBody, (const std::string& body), (override));
    MOCK_METHOD(bool, sendRequest,
                ((std::map<std::string, std::string> * headers), std::string* response_body),
                (override));
};

#endif  // MOCK_REQUEST_BUILDER_H