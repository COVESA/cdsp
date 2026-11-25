#ifndef MOCK_RDFOX_ADAPTER_H
#define MOCK_RDFOX_ADAPTER_H

#include <gmock/gmock.h>

#include "rdfox_adapter.h"

class MockRDFoxAdapter : public RDFoxAdapter {
   public:
    MockRDFoxAdapter(const ReasonerServerData& server_data) : RDFoxAdapter(server_data) {}

    MOCK_METHOD(std::unique_ptr<RequestBuilder>, createRequestBuilder, (), (const, override));
};
#endif  // MOCK_RDFOX_ADAPTER_H
