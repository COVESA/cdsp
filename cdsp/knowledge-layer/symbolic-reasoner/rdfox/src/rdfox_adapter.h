#ifndef RDFOX_ADAPTER_H
#define RDFOX_ADAPTER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <map>
#include <optional>
#include <string>
#include <tuple>

#include "request_builder.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class RDFoxAdapter {
   public:
    RDFoxAdapter(const std::string& host, const std::string& port, const std::string& auth_base64,
                 const std::string& data_store);

    void initialize();
    virtual bool loadData(const std::string& data, const std::string& content_type = "text/turtle");
    virtual std::string queryData(const std::string& sparql_query,
                                  const std::string& accept_type = "table/csv");
    virtual bool checkDataStore();
    bool deleteDataStore();

    // Cursor-related methods
    std::pair<std::string, std::string> createConnection();

    bool checkConnection(const std::string& connection_id);
    std::string createCursor(const std::string& connection_id, const std::string& auth_token,
                             const std::string& query);
    bool advanceCursor(const std::string& connection_id, const std::string& auth_token,
                       const std::string& cursor_id, const std::string& accept_type,
                       const std::string& operation, std::optional<int> limit,
                       std::string* response);

    bool deleteCursor(const std::string& connection_id, const std::string& cursor_id);

   protected:
    virtual std::unique_ptr<RequestBuilder> createRequestBuilder() const;

   private:
    std::string host_;
    std::string port_;
    std::string auth_header_base64_;
    std::string data_store_;
};

#endif  // RDFOX_ADAPTER_H