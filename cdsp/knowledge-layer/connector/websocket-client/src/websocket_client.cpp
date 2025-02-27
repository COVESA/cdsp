#include "websocket_client.h"

#include <iostream>
#include <variant>

#include "data_message.h"
#include "dto_to_bo.h"
#include "helper.h"
#include "real_websocket_connection.h"
#include "status_message.h"

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
 * @brief Creates a log message for a received error message.
 *
 * @param error The InternalErrorMessage object.
 * @return A string containing the formatted log message.
 */
std::string createLogInternalErrorMessage(const InternalErrorMessage& error) {
    std::string errorDetails;
    if (error.message.empty()) {
        return "Received message with error code " + std::to_string(error.errorCode) + " - (" +
               error.type + ")";
    }

    return "Received message with error code " + std::to_string(error.errorCode) + " - (" +
           error.type + "): " + error.message;
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

/**
 * @brief Initializes the WebSocket connection.
 *
 * This method checks if the connection is already established. If not, it creates a new
 * instance of RealWebSocketConnection using the io_context_ and a shared pointer to the
 * current WebSocketClient instance.
 */
void WebSocketClient::initializeConnection() {
    if (!connection_) {
        auto self = shared_from_this();
        connection_ = std::make_shared<RealWebSocketConnection>(io_context_, self);
    }
}

/**
 * @brief Starts the WebSocket client's execution flow.
 *
 * This method resolves the WebSocket server's host and port, and then runs the IO context
 * to process asynchronous events.
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

/**
 * @brief Initiates the WebSocket handshake process and subscribes to supported schema types.
 *
 * This function is called to perform the WebSocket handshake. If the handshake
 * fails, it logs the error and returns. Upon successful handshake, it subscribes
 * to all supported schema types defined in the configuration and retrieves all data
 * points for each schema type.
 *
 * @param ec The error code indicating the result of the handshake attempt.
 */
void WebSocketClient::handshake(boost::system::error_code ec) {
    if (ec) {
        return Fail(ec, "Handshake failed:");
    }

    for (const SchemaType& schema_type :
         init_config_.model_config.reasoner_settings.supported_schema_collections) {
        // Start subscribing to supported schemas
        // TODO: Refactor to subscribe to each node in the schema collection
        MessageHeader subscribe_header(init_config_.oid[schema_type], schema_type);
        MessageUtils::addMessageToQueue(SubscribeMessage(subscribe_header, {}),
                                        reply_messages_queue_);

        // Create a get message to retrieve all data points for the collection
        std::vector<Node> nodes;
        for (const auto& data_point : init_config_.model_config.system_data_points[schema_type]) {
            nodes.push_back(Node(data_point, std::nullopt, Metadata(),
                                 init_config_.model_config.system_data_points[schema_type]));
        }

        MessageHeader get_header(init_config_.oid[schema_type], schema_type);
        MessageUtils::addMessageToQueue(GetMessage(get_header, nodes), reply_messages_queue_);
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
    // TODO: Remove the sleep code: temporary flag to await the open session in the Information
    // Layer (Fix: ESSENCE-976)
    if (temporary_messaging_sleep_flag_) {
        temporary_messaging_sleep_flag_ = false;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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
 * This function processes the received message by parsing it and converting it to a DataMessage or
 * StatusMessage object.
 *
 * @param message The message to process.
 */
void WebSocketClient::processMessage(const std::shared_ptr<const std::string>& message) {
    response_messages_queue_.push_back(message);
    auto prio_message = *response_messages_queue_.front();
    response_messages_queue_.erase(response_messages_queue_.begin());

    const auto parsed_message = MessageUtils::displayAndParseMessage(prio_message);

    if (std::holds_alternative<InternalErrorMessage>(parsed_message)) {
        auto error = std::get<InternalErrorMessage>(parsed_message);
        throw std::runtime_error(createLogInternalErrorMessage(error));
    } else if (std::holds_alternative<StatusMessageDto>(parsed_message)) {
        StatusMessageDto status_message_dto = std::get<StatusMessageDto>(parsed_message);

        // Convert to BO and log
        try {
            StatusMessage status_message = DtoToBo::convert(status_message_dto);
            std::cout << "Status message received(" << status_message.getCode()
                      << "): " << status_message.getMessage() << std::endl
                      << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing status message: " << e.what() << std::endl;
        }
    } else {
        DataMessageDto data_message_dto = std::get<DataMessageDto>(parsed_message);

        try {
            // Convert to BO and process
            DataMessage data_message =
                DtoToBo::convert(data_message_dto, init_config_.model_config.system_data_points);
            triple_assembler_.transformMessageToRDFTriple(data_message);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing and transforming data message to RDF triple: " << e.what()
                      << std::endl;
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
 *
 * This function sends the first message in the reply_messages_queue_ to the WebSocket server.
 */
void WebSocketClient::writeReplyMessagesOnQueue() {
    if (reply_messages_queue_.empty()) {
        std::cerr << "No messages to send." << std::endl;
        return;
    }
    json reply_message = reply_messages_queue_.front();
    reply_messages_queue_.erase(reply_messages_queue_.begin());
    std::cout << "Sending queue message: " << reply_message.dump() << std::endl;
    sendMessage(reply_message);
}