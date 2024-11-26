#ifndef RDFOX_ADAPTER_H
#define RDFOX_ADAPTER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <optional>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class RDFoxAdapter {
   public:
    RDFoxAdapter(const std::string& host, const std::string& port, const std::string& auth_base64,
                 const std::string& data_store);

    void initialize();
    virtual bool loadData(const std::string& ttl_data);
    virtual std::string queryData(const std::string& sparql_query);
    virtual bool checkDataStore();
    bool deleteDataStore();
    virtual ~RDFoxAdapter() = default;

   protected:
    virtual bool sendRequest(http::verb method, const std::string& target, const std::string& body,
                             const std::string& contentType, const std::string& acceptType,
                             std::string& responseBody);

   private:
    std::string sendGetRequest(const std::string& target, const std::string& accept_type);
    std::optional<std::string> sendPostRequest(const std::string& target, const std::string& body,
                                               const std::string& content_type);

    std::string createErrorMessage(const std::string& error_msg, int error_code);
    std::string host_;
    std::string port_;
    std::string auth_header_base64_;
    std::string data_store_;
};

#endif  // RDFOX_ADAPTER_H