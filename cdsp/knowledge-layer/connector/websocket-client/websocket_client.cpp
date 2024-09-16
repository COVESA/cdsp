#include "websocket_client.hpp"

#include <iostream>

namespace {
void Fail(beast::error_code error_code, const std::string& what) {
    std::cerr << what << ": " << error_code.message() << "\n";
}

// [WIP]: createTestMessage is used to test the first knowledge layer implementation. The
// subscription messaging should manage dynamically.
json createTestMessage() {
    nlohmann::json message;
    message["type"] = "subscribe";
    message["tree"] = "VSS";
    message["id"] = "WBY11CF080CH470711";
    message["uuid"] = "Some_UUID";
    return message;
}
}  // namespace

WebSocketClient::WebSocketClient(const std::string& host, const std::string& port)
    : resolver_(io_context_), ws_(io_context_), server_uri_(host), server_port_(port) {}

void WebSocketClient::run() {
    resolver_.async_resolve(
        server_uri_, server_port_,
        beast::bind_front_handler(&WebSocketClient::onResolve, shared_from_this()));

    // Run the io_context to process asynchronous events
    io_context_.run();
}

void WebSocketClient::onResolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        return Fail(ec, "Resolve failed:");
    }
    net::async_connect(ws_.next_layer(), results.begin(), results.end(),
                       beast::bind_front_handler(&WebSocketClient::onConnect, shared_from_this()));
}

void WebSocketClient::onConnect(boost::system::error_code ec,
                                tcp::resolver::iterator /*end_point*/) {
    if (ec) {
        return Fail(ec, "Connection failed:");
    }

    ws_.async_handshake(server_uri_, "/",
                        beast::bind_front_handler(&WebSocketClient::handshake, shared_from_this()));
}

void WebSocketClient::handshake(beast::error_code ec) {
    if (ec) {
        return Fail(ec, "Handshake failed:");
    }
    std::cout << "Connected to WebSocket server: " << server_uri_ << std::endl;
    json message = createTestMessage();
    sendMessage(message);
}

void WebSocketClient::receiveMessage() {
    ws_.async_read(
        buffer_, beast::bind_front_handler(&WebSocketClient::onReceiveMessage, shared_from_this()));
}

void WebSocketClient::onReceiveMessage(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        return Fail(ec, "read");
    }

    std::string received_message = boost::beast::buffers_to_string(buffer_.data());
    json response = json::parse(received_message);
    std::cout << "Received message: " << response.dump(4) << std::endl;
    buffer_.consume(bytes_transferred);  // Clear the buffer for the next message
    receiveMessage();                    // Continue receiving
}

void WebSocketClient::sendMessage(const json& message) {
    std::cout << "Sending message send:" << message.dump() << std::endl;
    ws_.async_write(net::buffer(message.dump()),
                    beast::bind_front_handler(&WebSocketClient::onSendMessage, shared_from_this()));
}

void WebSocketClient::onSendMessage(boost::system::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        return Fail(ec, "write");
    }
    std::cout << "Message send! " << bytes_transferred << " bytes transferred" << std::endl;
    receiveMessage();
}