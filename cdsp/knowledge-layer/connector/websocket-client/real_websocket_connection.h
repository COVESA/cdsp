#ifndef REAL_WEBSOCKET_CONNECTION_H
#define REAL_WEBSOCKET_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/system/error_code.hpp>
#include <memory>
#include <nlohmann/json.hpp>

#include "websocket_client.h"
#include "websocket_interface.h"

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using json = nlohmann::json;

class RealWebSocketConnection : public WebSocketClientInterface {
   public:
    RealWebSocketConnection(net::io_context& io_context, std::weak_ptr<WebSocketClient> client);

    void asyncResolve(const std::string& host, const std::string& port) override;
    void asyncConnect() override;
    void asyncHandshake() override;
    void asyncWrite(const json& message) override;
    void asyncRead() override;

    std::string getReceivedMessage() override;
    void consumeBuffer(std::size_t bytes) override;

   private:
    tcp::resolver resolver_;
    tcp::resolver::results_type resolve_results_;
    beast::websocket::stream<tcp::socket> ws_;
    std::weak_ptr<WebSocketClient> client_;
    beast::flat_buffer buffer_;

    void onResolve(beast::error_code ec, tcp::resolver::results_type results);
    void Fail(beast::error_code ec, const char* what);
};

#endif  // REAL_WEBSOCKET_CONNECTION_H
