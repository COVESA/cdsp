#include "websocket_client.h"

#include <iostream>

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
}  // namespace

/**
 * @brief Constructs a WebSocketClient object with the specified initialization configuration and
 * connection.
 *
 * This constructor initializes the WebSocketClient with the given configuration and connection
 * interface. It sets up the RDFox adapter and triple assembler using the provided configuration and
 * initializes them.
 *
 * @param system_config The initialization configuration containing settings for the
 * WebSocketClient.
 * @param connection A shared pointer to a WebSocketClientInterface, representing the connection to
 * be used.
 */
WebSocketClient::WebSocketClient(SystemConfig system_config,
                                 std::shared_ptr<ModelConfig> model_config,
                                 std::shared_ptr<ReasonerService> reasoner_service,
                                 std::shared_ptr<WebSocketClientInterface> connection)
    : system_config_(std::move(system_config)),
      reasoner_service_(std::move(reasoner_service)),
      model_config_(std::move(model_config)),
      connection_(std::move(connection)),
      triple_assembler_(model_config_, *reasoner_service_, file_handler_, triple_writer_),
      reasoner_query_service_(std::make_shared<ReasoningQueryService>(reasoner_service_)) {
    triple_assembler_.initialize();
}

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
    connection_->asyncResolve(system_config_.websocket_server.host,
                              system_config_.websocket_server.port);
    // Run the IO context to process asynchronous events
    io_context_.run();
}

/**
 * @brief Provides access to the client's configuration.
 *
 * @return The current SystemConfig object.
 */
const SystemConfig& WebSocketClient::getInitConfig() const { return system_config_; }

// Callbacks for connection lifecycle and message handling
void WebSocketClient::onConnect(boost::system::error_code error_code,
                                const boost::asio::ip::tcp::endpoint& endpoint) {
    if (error_code) {
        Fail(error_code, " - Connection failed:");
        return;
    }

    std::cout << " - Connected to Websocket Server: " << endpoint.address() << ":"
              << endpoint.port() << "\n";
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
void WebSocketClient::handshake(boost::system::error_code error_code) {
    if (error_code) {
        return Fail(error_code, " - Handshake failed:");
    }

    for (const SchemaType& schema_type :
         model_config_->getReasonerSettings().getSupportedSchemaCollections()) {
        // Start subscribing to supported schemas
        const auto object_id = model_config_->getObjectId().at(schema_type);
        auto data_point_list = model_config_->getInputs().at(schema_type);

        MessageService::createAndQueueSubscribeMessage(object_id, schema_type,
                                                       data_point_list.subscribe,
                                                       *request_registry_, reply_messages_queue_);
    }

    std::cout << " - Handshake succeeded!\n\n";

    writeReplyMessagesOnQueue();
}

/**
 * @brief Sends a JSON message to the WebSocket server.
 *
 * @param message The JSON message to be sent.
 */
void WebSocketClient::sendMessage(const json& message) { connection_->asyncWrite(message); }

void WebSocketClient::onSendMessage(boost::system::error_code error_code,
                                    std::size_t bytes_transferred) {
    if (error_code) {
        Fail(error_code, "write");
        return;
    }
    std::cout << "Message sent! " << bytes_transferred << " bytes transferred\n\n";
    connection_->asyncRead();
}

void WebSocketClient::onReceiveMessage(boost::beast::error_code error_code,
                                       std::size_t bytes_transferred) {
    if (error_code) {
        Fail(error_code, "read");
        return;
    }

    const auto received_message = std::make_shared<std::string>(connection_->getReceivedMessage());
    connection_->consumeBuffer(bytes_transferred);  // Clear the buffer for the next message
    processMessage(received_message);
}

/**
 * @brief Processes an incoming WebSocket message.
 *
 * This method handles an incoming message by adding it to the response message queue,
 * extracting the highest priority message, and attempting to transform it into a reasoning triple
 * if it contains valid data. If there are reply messages queued, it writes them to the queue;
 * otherwise, it initiates an asynchronous read operation on the connection.
 *
 * @param message A shared pointer to the incoming message string to be processed.
 */
void WebSocketClient::processMessage(const std::shared_ptr<const std::string>& message) {
    response_messages_queue_.push_back(message);
    auto prio_message = *response_messages_queue_.front();
    response_messages_queue_.erase(response_messages_queue_.begin());

    // Attempt to extract a data message from the priority message
    // or process the status message and log any errors if present
    const auto data_message =
        MessageService::getDataOrProcessStatusFromMessage(prio_message, *request_registry_);

    // Process the data message if it is valid
    if (data_message.has_value()) {
        // If the message is a regular data message, process it as a triple and
        // handle the reasoning queries
        triple_assembler_.transformMessageToTriple(data_message.value());

        // Process reasoning query
        for (const auto& reasoning_output_query : model_config_->getReasoningOutputQueries()) {
            try {
                json result = reasoner_query_service_->processReasoningQuery(
                    reasoning_output_query,
                    model_config_->getReasonerSettings().isIsAiReasonerInferenceResults(),
                    model_config_->getOutput() + "/reasoning_output/");

                if (!result.empty()) {
                    auto data_point_list = model_config_->getInputs();

                    MessageService::createAndQueueSetMessage(
                        model_config_->getObjectId(), result, *request_registry_,
                        reply_messages_queue_, system_config_.reasoner_server.origin_system_name);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing reasoning query: " << e.what() << std::endl;
            }
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
    std::cout << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
              << " Sending queue message:\n"
              << reply_message.dump() << std::endl;
    sendMessage(reply_message);
}
