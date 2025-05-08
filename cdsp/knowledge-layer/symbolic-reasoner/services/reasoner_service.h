#ifndef REASONER_SERVICE_H
#define REASONER_SERVICE_H

#include <iostream>

#include "data_types.h"
#include "i_reasoner_adapter.h"
#include "memory"

class ReasonerService {
   public:
    explicit ReasonerService(std::shared_ptr<IReasonerAdapter> adapter, const bool reset_datastore)
        : adapter_(std::move(adapter)) {
        if (reset_datastore) {
            std::cout << "Deleting the datastore...\n";
            deleteDataStore();
            std::cout << std::endl;
        }
        adapter_->initialize();
    }

    virtual bool checkDataStore() { return adapter_->checkDataStore(); }

    virtual bool loadData(const std::string& data, const ReasonerSyntaxType& content_type) {
        const std::string content_type_str = reasonerSyntaxTypeToContentType(content_type);
        return adapter_->loadData(data, content_type_str);
    }

    virtual bool loadRules(const std::string& rules, const RuleLanguageType& content_type) {
        const std::string content_type_str = ruleLanguageTypeToContentType(content_type);
        return adapter_->loadData(rules, content_type_str);
    }

    virtual std::string queryData(
        const std::string& query, const QueryLanguageType& query_language_type,
        const DataQueryAcceptType& accept_type = DataQueryAcceptType::TEXT_TSV) {
        return adapter_->queryData(query, query_language_type, accept_type);
    }

    virtual bool deleteDataStore() { return adapter_->deleteDataStore(); }

   private:
    std::shared_ptr<IReasonerAdapter> adapter_;
};

#endif  // REASONER_SERVICE_H