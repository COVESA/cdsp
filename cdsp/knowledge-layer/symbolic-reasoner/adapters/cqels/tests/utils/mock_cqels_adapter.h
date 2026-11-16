#ifndef MOCK_CQELS_ADAPTER_H
#define MOCK_CQELS_ADAPTER_H

#include <gmock/gmock.h>

#include "cqels_adapter.h"

class MockCQELSAdapter : public CQELSAdapter {
   public:
    MockCQELSAdapter(const ReasonerServerData& server_data) : CQELSAdapter(server_data) {}

    MOCK_METHOD(std::unique_ptr<RequestBuilder>, createRequestBuilder, (), (const, override));
};
#endif  // MOCK_CQELS_ADAPTER_H
