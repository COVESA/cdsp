#ifndef MOCK_RDF4J_ADAPTER_H
#define MOCK_RDF4J_ADAPTER_H

#include <gmock/gmock.h>

#include "rdf4j_adapter.h"

class MockRDF4JAdapter : public RDF4JAdapter {
   public:
    MockRDF4JAdapter(const ReasonerServerData& server_data) : RDF4JAdapter(server_data) {}

    MOCK_METHOD(std::unique_ptr<RequestBuilder>, createRequestBuilder, (), (const, override));
};
#endif  // MOCK_RDF4J_ADAPTER_H
