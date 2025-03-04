#ifndef MOCK_REASONER_ADAPTER_H
#define MOCK_REASONER_ADAPTER_H

#include <gmock/gmock.h>

#include "i_reasoner_adapter.h"

class MockReasonerAdapter : public IReasonerAdapter {
   public:
    MOCK_METHOD(void, initialize, (), (override));
    MOCK_METHOD(bool, checkDataStore, (), (override));
    MOCK_METHOD(std::string, queryData,
                (const std::string& query, const QueryLanguageType& query_language_type,
                 const DataQueryAcceptType& accept_type),
                (override));
    MOCK_METHOD(bool, loadData, (const std::string& data, const std::string& content_type),
                (override));
    MOCK_METHOD(bool, deleteDataStore, (), (override));
};
#endif  // MOCK_REASONER_ADAPTER_H