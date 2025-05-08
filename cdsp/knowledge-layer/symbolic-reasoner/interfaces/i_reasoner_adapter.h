#ifndef I_REASONER_ADAPTER_H
#define I_REASONER_ADAPTER_H

#include <optional>
#include <string>
#include <tuple>

#include "data_types.h"

class IReasonerAdapter {
   public:
    virtual void initialize() = 0;
    virtual bool loadData(const std::string& data, const std::string& content_type) = 0;
    virtual std::string queryData(const std::string& query,
                                  const QueryLanguageType& query_language_type,
                                  const DataQueryAcceptType& accept_type) = 0;
    virtual bool checkDataStore() = 0;
    virtual bool deleteDataStore() = 0;
};

#endif  // I_REASONER_ADAPTER_H