#include "real_websocket_connection.h"

#include <iostream>

RealWebSocketConnection::RealWebSocketConnection(net::io_context& io_context,
                                                 std::weak_ptr<WebSocketClient> client)
    : resolver_(io_context), ws_(io_context), client_(std::move(client)) {}

void RealWebSocketConnection::asyncResolve(const std::string& host, const std::string& port) {
    resolver_.async_resolve(host, port,
                            beast::bind_front_handler(&RealWebSocketConnection::onResolve, this));
}

void RealWebSocketConnection::onResolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        return Fail(ec, "Resolve failed");
    }
    resolve_results_ = results;
    asyncConnect();
}

void RealWebSocketConnection::asyncConnect() {
    if (auto client = client_.lock()) {
        auto shared_client = client;  // Ensure shared_ptr is captured
        net::async_connect(ws_.next_layer(), resolve_results_,
                           [shared_client](boost::system::error_code ec,
                                           const boost::asio::ip::tcp::endpoint& endpoint) {
                               if (shared_client) {
                                   shared_client->onConnect(ec, endpoint);
                               } else {
                                   std::cerr << "WebSocketClient instance no longer exists.\n";
                               }
                           });
    } else {
        std::cerr << "Failed to lock WebSocketClient. Client may have been destroyed.\n";
    }
}

void RealWebSocketConnection::asyncHandshake() {
    if (auto client = client_.lock()) {
        auto shared_client = client;  // Ensure shared_ptr is captured
        ws_.async_handshake(
            client->getInitConfig().websocket_server.host,
            "/" + client->getInitConfig().websocket_server.target,
            [shared_client](boost::system::error_code ec) { shared_client->handshake(ec); });
    } else {
        std::cerr << "Failed to lock WebSocketClient. Client may have been destroyed.\n";
    }
}

void RealWebSocketConnection::asyncWrite(const json& message) {
    if (auto client = client_.lock()) {
        auto shared_client = client;  // Ensure shared_ptr is captured
        ws_.async_write(
            net::buffer(message.dump()),
            [shared_client](boost::system::error_code ec, std::size_t bytes_transferred) {
                shared_client->onSendMessage(ec, bytes_transferred);
            });
    } else {
        std::cerr << "Failed to lock WebSocketClient. Client may have been destroyed.\n";
    }
}

void RealWebSocketConnection::asyncRead() {
    if (auto client = client_.lock()) {
        auto shared_client = client;  // Ensure shared_ptr is captured
        ws_.async_read(buffer_,
                       [shared_client](boost::beast::error_code ec, std::size_t bytes_transferred) {
                           shared_client->onReceiveMessage(ec, bytes_transferred);
                       });
    } else {
        std::cerr << "Failed to lock WebSocketClient. Client may have been destroyed.\n";
    }
}

void RealWebSocketConnection::Fail(beast::error_code ec, const char* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

std::string RealWebSocketConnection::getReceivedMessage() {
    return boost::beast::buffers_to_string(buffer_.data());
}

void RealWebSocketConnection::consumeBuffer(std::size_t bytes) { buffer_.consume(bytes); }