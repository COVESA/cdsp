#include "websocket_client.h"

#include <iostream>
#include <variant>

#include "helper.h"

namespace {
void Fail(beast::error_code error_code, const std::string& what) {
    std::cerr << what << ": " << error_code.message() << "\n";
}

std::string createLogErrorMessage(ErrorMessage& error) {
    std::string errorDetails;
    if (std::holds_alternative<std::string>(error.error)) {
        errorDetails = std::get<std::string>(error.error);
    } else {
        const std::vector<ErrorNode>& nodes = std::get<std::vector<ErrorNode>>(error.error);
        for (const auto& node : nodes) {
            errorDetails += "\n  - Node: " + node.name + ": " + node.status;
        }
    }

    return "Received message with error code " + std::to_string(error.errorCode) + " - (" +
           error.type + ") " + errorDetails;
}
}  // namespace

WebSocketClient::WebSocketClient(const InitConfig& init_config)
    : resolver_(io_context_), ws_(io_context_), init_config_(init_config) {}

void WebSocketClient::run() {
    resolver_.async_resolve(
        init_config_.websocket_server.host, init_config_.websocket_server.port,
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

    ws_.async_handshake(init_config_.websocket_server.host, "/",
                        beast::bind_front_handler(&WebSocketClient::handshake, shared_from_this()));
}

void WebSocketClient::handshake(beast::error_code ec) {
    if (ec) {
        return Fail(ec, "Handshake failed:");
    }
    std::cout << "Connected to WebSocket server: " << init_config_.websocket_server.host
              << std::endl
              << std::endl;

    for (const std::string& tree_type :
         init_config_.model_config.reasoner_settings.supported_tree_types) {
        createSubscription(init_config_.uuid, init_config_.oid, tree_type, reply_messages_queue_);
        createReadMessage(init_config_.uuid, tree_type, init_config_.oid,
                          init_config_.model_config.system_data_points[toLowercase(tree_type)],
                          reply_messages_queue_);
    }

    writeReplyMessagesOnQueue();
}

void WebSocketClient::sendMessage(const json& message) {
    ws_.async_write(net::buffer(message.dump()),
                    beast::bind_front_handler(&WebSocketClient::onSendMessage, shared_from_this()));
}

void WebSocketClient::onSendMessage(boost::system::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        return Fail(ec, "write");
    }
    std::cout << "Message send! " << bytes_transferred << " bytes transferred" << std::endl
              << std::endl;
    receiveMessage();
}

void WebSocketClient::receiveMessage() {
    ws_.async_read(
        buffer_, beast::bind_front_handler(&WebSocketClient::onReceiveMessage, shared_from_this()));
}

void WebSocketClient::onReceiveMessage(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        return Fail(ec, "read");
    }

    const auto received_message =
        std::make_shared<std::string>(boost::beast::buffers_to_string(buffer_.data()));
    buffer_.consume(bytes_transferred);  // Clear the buffer for the next message
    processMessage(received_message);
}

void WebSocketClient::processMessage(std::shared_ptr<std::string const> const& message) {
    boost::asio::post(
        ws_.get_executor(),
        beast::bind_front_handler(&WebSocketClient::onProcessMessage, shared_from_this(), message));
}

void WebSocketClient::onProcessMessage(std::shared_ptr<std::string const> const& message) {
    response_messages_queue_.push_back(message);

    std::string prio_message = *response_messages_queue_.begin()->get();
    response_messages_queue_.erase(response_messages_queue_.begin());

    const auto parsed_message = displayAndParseMessage(prio_message);

    if (std::holds_alternative<ErrorMessage>(parsed_message)) {
        ErrorMessage error = std::get<ErrorMessage>(parsed_message);
        throw std::runtime_error(createLogErrorMessage(error));
    }

    DataMessage data_message = std::get<DataMessage>(parsed_message);

    if (!reply_messages_queue_.empty()) {
        writeReplyMessagesOnQueue();
    } else {
        receiveMessage();
    }
}

void WebSocketClient::writeReplyMessagesOnQueue() {
    json reply_message = reply_messages_queue_.front();
    reply_messages_queue_.erase(reply_messages_queue_.begin());
    std::cout << "Sending queue message: " << reply_message.dump() << std::endl;
    sendMessage(reply_message);
}
