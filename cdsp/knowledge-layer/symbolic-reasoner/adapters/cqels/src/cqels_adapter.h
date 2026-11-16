#ifndef CQELS_ADAPTER_H
#define CQELS_ADAPTER_H

#include <chrono>
#include <map>
#include <set>
#include <string>

#include "data_types.h"
#include "i_reasoner_adapter.h"
#include "request_builder.h"

class CQELSAdapter : public IReasonerAdapter {
   public:
    explicit CQELSAdapter(const ReasonerServerData& server_data);

    virtual void initialize();
    virtual bool loadData(const std::string& data, const std::string& content_type = "text/turtle");
    virtual bool loadObservations(const std::string& data, const std::string& content_type,
                                  std::chrono::system_clock::time_point observation_time);
    virtual bool loadRules(const std::string& rules, const std::string& content_type);
    virtual std::string queryData(
        const std::string& query,
        const QueryLanguageType& query_language_type = QueryLanguageType::SPARQL,
        const DataQueryAcceptType& accept_type = DataQueryAcceptType::TEXT_TSV);
    virtual bool checkDataStore();
    bool deleteDataStore();

   protected:
    virtual std::unique_ptr<RequestBuilder> createRequestBuilder() const;

   private:
    std::string host_;
    std::string port_;
    std::string auth_header_base64_;
    std::string data_store_;
    std::map<std::string, std::string> query_ids_;
    std::set<std::string> streams_;
};

#endif  // CQELS_ADAPTER_H
