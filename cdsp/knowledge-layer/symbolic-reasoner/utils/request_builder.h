#ifndef REQUEST_BUILDER_H
#define REQUEST_BUILDER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <map>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class RequestBuilder {
   public:
    RequestBuilder(const std::string& host, const std::string& port,
                   const std::string& auth_base64);

    virtual RequestBuilder& setAuthorization(const std::string& auth_header_base64);
    virtual RequestBuilder& setMethod(http::verb method);
    virtual RequestBuilder& setTarget(const std::string& target);
    virtual RequestBuilder& setContentType(const std::string& content_type);
    virtual RequestBuilder& setAcceptType(const std::string& accept_type);
    virtual RequestBuilder& setBody(const std::string& body);
    virtual bool sendRequest(std::map<std::string, std::string>* headers = nullptr,
                             std::string* response_body = nullptr);
    virtual ~RequestBuilder() = default;

   private:
    http::verb method_;
    std::string target_;
    std::string host_;
    std::string auth_header_base64_;
    std::string content_type_;
    std::string accept_type_;
    std::string body_;
    std::string port_;

    std::string createErrorMessage(const std::string& error_msg, int error_code);

    bool validateRequiredFields();
};

#endif  // REQUEST_BUILDER_H