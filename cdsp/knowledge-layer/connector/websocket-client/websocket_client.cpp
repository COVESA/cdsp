#include "websocket_client.h"

#include <iostream>
#include <variant>

#include "helper.h"
#include "real_websocket_connection.h"

namespace {
/**
 * @brief Logs errors encountered during asynchronous operations.
 *
 * @param ec The Boost.Beast error code.
 * @param what Description of the operation that failed.
 */
void Fail(const boost::system::error_code& ec, const std::string& what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

/**
 * @brief Creates a log message for a received category message.
 *
 * @param categoryMessage The CategoryMessage object.
 * @return A string containing the formatted log message.
 */
std::string createLogCategoryMessage(const CategoryMessage& categoryMessage) {
    return "Received a message category: " + categoryMessage.category +
           "\nMessage: " + categoryMessage.message +
           "\nStatus Code: " + std::to_string(categoryMessage.statusCode);
}

/**
 * @brief Creates a log message for a received error message.
 *
 * @param error The ErrorMessage object.
 * @return A string containing the formatted log message.
 */
std::string createLogErrorMessage(const ErrorMessage& error) {
    std::string errorDetails;
    if (std::holds_alternative<std::string>(error.error)) {
        errorDetails = std::get<std::string>(error.error);
    } else {
        const auto& nodes = std::get<std::vector<ErrorNode>>(error.error);
        for (const auto& node : nodes) {
            errorDetails += "\n  - Node: " + node.name + ": " + node.status;
        }
    }

    return "Received message with error code " + std::to_string(error.errorCode) + " - (" +
           error.type + ") " + errorDetails;
}
}  // namespace

/**
 * @brief Constructor for the WebSocketClient.
 *
 * @param init_config Configuration for initializing the client.
 * @param triple_assembler Reference to the TripleAssembler for processing messages.
 * @param connection Optional custom WebSocket connection implementation.
 */
WebSocketClient::WebSocketClient(const InitConfig& init_config, TripleAssembler& triple_assembler,
                                 std::shared_ptr<WebSocketClientInterface> connection)
    : init_config_(init_config),
      triple_assembler_(triple_assembler),
      io_context_(),
      connection_(std::move(connection)) {}

void WebSocketClient::initializeConnection() {
    if (!connection_) {
        auto self = shared_from_this();
        connection_ = std::make_shared<RealWebSocketConnection>(io_context_, self);
    }
}

/**
 * @brief Starts the WebSocket client's execution flow.
 */
void WebSocketClient::run() {
    connection_->asyncResolve(init_config_.websocket_server.host,
                              init_config_.websocket_server.port);
    // Run the IO context to process asynchronous events
    io_context_.run();
}

/**
 * @brief Provides access to the client's configuration.
 *
 * @return The current InitConfig object.
 */
const InitConfig& WebSocketClient::getInitConfig() const { return init_config_; }

// Callbacks for connection lifecycle and message handling
void WebSocketClient::onConnect(boost::system::error_code ec,
                                const boost::asio::ip::tcp::endpoint& endpoint) {
    if (ec) {
        return Fail(ec, "Connection failed:");
    }

    std::cout << "Connected to WebSocket server: " << endpoint.address() << ":" << endpoint.port()
              << "\n";
    connection_->asyncHandshake();
}

void WebSocketClient::handshake(boost::system::error_code ec) {
    if (ec) {
        return Fail(ec, "Handshake failed:");
    }

    for (const std::string& tree_type :
         init_config_.model_config.reasoner_settings.supported_tree_types) {
        MessageUtils::createSubscription(init_config_.uuid, init_config_.oid, tree_type,
                                         reply_messages_queue_);
        MessageUtils::createReadMessage(
            init_config_.uuid, tree_type, init_config_.oid,
            init_config_.model_config.system_data_points[Helper::toLowerCase(tree_type)],
            reply_messages_queue_);
    }

    writeReplyMessagesOnQueue();
}

/**
 * @brief Sends a JSON message to the WebSocket server.
 *
 * @param message The JSON message to be sent.
 */
void WebSocketClient::sendMessage(const json& message) { connection_->asyncWrite(message); }

void WebSocketClient::onSendMessage(boost::system::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        return Fail(ec, "write");
    }
    std::cout << "Message sent! " << bytes_transferred << " bytes transferred\n\n";
    connection_->asyncRead();
}

void WebSocketClient::onReceiveMessage(boost::beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        return Fail(ec, "read");
    }

    const auto received_message = std::make_shared<std::string>(connection_->getReceivedMessage());
    connection_->consumeBuffer(bytes_transferred);  // Clear the buffer for the next message
    processMessage(received_message);
}

/**
 * @brief Processes a message received from the WebSocket server.
 *
 * @param message The message to process.
 */
void WebSocketClient::processMessage(const std::shared_ptr<const std::string>& message) {
    response_messages_queue_.push_back(message);
    auto prio_message = *response_messages_queue_.front();
    response_messages_queue_.erase(response_messages_queue_.begin());

    const auto parsed_message = MessageUtils::displayAndParseMessage(prio_message);

    if (std::holds_alternative<ErrorMessage>(parsed_message)) {
        auto error = std::get<ErrorMessage>(parsed_message);
        throw std::runtime_error(createLogErrorMessage(error));
    } else if (std::holds_alternative<CategoryMessage>(parsed_message)) {
        auto error = std::get<CategoryMessage>(parsed_message);
        throw std::runtime_error(createLogCategoryMessage(error));
    } else {
        DataMessage data_message = std::get<DataMessage>(parsed_message);
        try {
            triple_assembler_.transformMessageToRDFTriple(data_message);
        } catch (const std::exception& e) {
            std::cerr << "Error transforming message to RDF triple: " << e.what() << std::endl;
        }
    }

    if (!reply_messages_queue_.empty()) {
        writeReplyMessagesOnQueue();
    } else {
        connection_->asyncRead();
    }
}

/**
 * @brief Sends messages queued in the reply_messages_queue_.
 */
void WebSocketClient::writeReplyMessagesOnQueue() {
    json reply_message = reply_messages_queue_.front();
    reply_messages_queue_.erase(reply_messages_queue_.begin());
    std::cout << "Sending queue message: " << reply_message.dump() << std::endl;
    sendMessage(reply_message);
}