#ifndef MOCK_REASONER_SERVICE_H
#define MOCK_REASONER_SERVICE_H

#include <gmock/gmock.h>

#include "reasoner_service.h"

class MockReasonerService : public ReasonerService {
   public:
    MockReasonerService(std::shared_ptr<IReasonerAdapter> adapter)
        : ReasonerService(adapter, false) {}

    MOCK_METHOD(bool, checkDataStore, (), (override));
    MOCK_METHOD(bool, loadData, (const std::string& data, const ReasonerSyntaxType& content_type),
                (override));
    MOCK_METHOD(bool, loadRules, (const std::string& rules, const RuleLanguageType& content_type),
                (override));
    MOCK_METHOD(std::string, queryData,
                (const std::string& query, const QueryLanguageType& query_language_type,
                 const DataQueryAcceptType& accept_type),
                (override));
    MOCK_METHOD(bool, deleteDataStore, (), (override));
};

#endif  // MOCK_REASONER_SERVICE_H