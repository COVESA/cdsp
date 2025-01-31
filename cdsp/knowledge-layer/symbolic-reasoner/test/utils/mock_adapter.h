#ifndef MOCK_ADAPTER_H
#define MOCK_ADAPTER_H

#include "mock_request_builder.h"
#include "rdfox_adapter.h"

class MockAdapter : public RDFoxAdapter {
   public:
    MockAdapter(const std::string& host, const std::string& port, const std::string& auth_base64,
                const std::string& data_store)
        : RDFoxAdapter(host, port, auth_base64, data_store) {}

    MOCK_METHOD(std::unique_ptr<RequestBuilder>, createRequestBuilder, (), (const, override));
    MOCK_METHOD(bool, checkDataStore, (), (override));
    MOCK_METHOD(std::string, queryData,
                (const std::string& sparql_query, const std::string& accept_type), (override));
    MOCK_METHOD(bool, loadData, (const std::string& ttl_data, const std::string& content_type),
                (override));
};
#endif  // MOCK_ADAPTER_H