#ifndef RDF4J_ADAPTER_H
#define RDF4J_ADAPTER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <map>
#include <optional>
#include <string>
#include <tuple>

#include "data_types.h"
#include "i_reasoner_adapter.h"
#include "request_builder.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class RDF4JAdapter : public IReasonerAdapter {
   public:
    explicit RDF4JAdapter(const ReasonerServerData& server_data);

    virtual void initialize();
    virtual bool loadData(const std::string& data, const std::string& content_type = "text/turtle");
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
};

#endif  // RDF4J_ADAPTER_H