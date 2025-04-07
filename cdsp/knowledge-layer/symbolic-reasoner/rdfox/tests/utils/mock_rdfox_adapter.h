#ifndef MOCK_RDFOX_ADAPTER_H
#define MOCK_RDFOX_ADAPTER_H

#include "mock_request_builder.h"
#include "rdfox_adapter.h"

class MockRDFoxAdapter : public RDFoxAdapter {
   public:
    MockRDFoxAdapter(const ReasonerServerData& server_data) : RDFoxAdapter(server_data) {}

    MOCK_METHOD(void, initialize, (), (override));
    MOCK_METHOD(std::unique_ptr<RequestBuilder>, createRequestBuilder, (), (const, override));
    MOCK_METHOD(bool, checkDataStore, (), (override));
    MOCK_METHOD(std::string, queryData,
                (const std::string& sparql_query, const QueryLanguageType& query_language_type,
                 const DataQueryAcceptType& accept_type),
                (override));
    MOCK_METHOD(bool, loadData, (const std::string& data, const std::string& content_type),
                (override));
};
#endif  // MOCK_RDFOX_ADAPTER_H