#include "websocket_client_base_integration_test.h"

#include <iostream>

#include "vin_utils.h"

void WebSocketClientBaseIntegrationTest::SetUp() {
    const std::string VIN = VinUtils::getRandomVinString();
    init_config_ = InitConfigFixture::getValidInitConfig(VIN);

    rdfox_adapter_ = std::make_shared<RDFoxAdapter>(
        init_config_.rdfox_server.host, init_config_.rdfox_server.port,
        init_config_.rdfox_server.auth_base64, init_config_.rdfox_server.data_store.value());
    rdfox_adapter_->initialize();

    triple_assembler_ = std::make_shared<TripleAssembler>(
        init_config_.model_config, *rdfox_adapter_, file_handler_, triple_writer_);
    triple_assembler_->initialize();

    mock_connection_ = std::make_shared<MockWebSocketConnection>();

    client_ = std::make_shared<WebSocketClient>(init_config_, *triple_assembler_, mock_connection_);
}

void WebSocketClientBaseIntegrationTest::TearDown() {
    ASSERT_TRUE(rdfox_adapter_->deleteDataStore()) << "Failed to clean up the test data store.";
}

void WebSocketClientBaseIntegrationTest::mockWebSocketBehavior() {
    mock_connection_->setOnResolveCallback([&]() {
        std::cout << "Mock: Resolved WebSocket server." << std::endl;
        mock_connection_->asyncConnect();
    });

    mock_connection_->setOnConnectCallback([&]() {
        std::cout << "Mock: Connected to WebSocket server." << std::endl;
        mock_connection_->asyncHandshake();
    });

    mock_connection_->setOnHandshakeCallback([&]() {
        std::cout << "Mock: Handshake completed." << std::endl;
        mock_connection_->asyncRead();
    });

    mock_connection_->setOnReadCallback([&]() {
        std::cout << "Mock: Read message from server." << std::endl;
        client_->onReceiveMessage({}, mock_connection_->getReceivedMessage().size());
    });

    client_->run();
}

/**
 * @brief Creates a JSON message containing data and metadata for a WebSocket client.
 *
 * This function constructs a JSON message string that includes data from a list of nodes
 * and associated metadata. The message is structured to include the type, instance, schema,
 * and metadata with timestamps for each node.
 *
 * @param vin The vehicle identification number (VIN) associated with the data.
 * @param nodes A vector of Node objects, each containing data and metadata.
 * @return A JSON-formatted string representing the data and metadata.
 */
const std::string WebSocketClientBaseIntegrationTest::createDataJsonMessage(
    const std::string& vin, const std::vector<MessageNodeData>& nodes) {
    std::ostringstream message;
    message << R"({
        "type": "data",
        "data": {)";

    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        message << R"(")" << node.name << R"(": )" << node.value;

        if (i < nodes.size() - 1) {
            message << ",";
        }
    }

    message << R"(},
        "instance": ")"
            << vin << R"(",
        "schema": "Vehicle",
        "metadata": {)";

    bool first_metadata = true;
    for (const auto& node : nodes) {
        bool has_generated = node.metadata.getGenerated().has_value();
        bool has_received = node.metadata.getReceived().time_since_epoch().count() > 0;

        if (has_generated || has_received) {
            if (!first_metadata) {
                message << ",";
            }
            first_metadata = false;

            message << R"(")" << node.name << R"(": {
            "timestamps": {)";

            bool first_timestamp = true;

            if (has_generated) {
                if (!first_timestamp) {
                    message << ",";
                }
                first_timestamp = false;

                auto generated_time = node.metadata.getGenerated().value();
                message << R"("generated": {
                "seconds": )"
                        << std::chrono::duration_cast<std::chrono::seconds>(
                               generated_time.time_since_epoch())
                               .count()
                        << R"(,
                "nanoseconds": )"
                        << std::chrono::duration_cast<std::chrono::nanoseconds>(
                               generated_time.time_since_epoch())
                                   .count() %
                               1000000000
                        << R"(})";
            }

            if (has_received) {
                if (!first_timestamp) {
                    message << ",";
                }

                auto received_time = node.metadata.getReceived();
                message << R"("received": {
                "seconds": )"
                        << std::chrono::duration_cast<std::chrono::seconds>(
                               received_time.time_since_epoch())
                               .count()
                        << R"(,
                "nanoseconds": )"
                        << std::chrono::duration_cast<std::chrono::nanoseconds>(
                               received_time.time_since_epoch())
                                   .count() %
                               1000000000
                        << R"(})";
            }

            message << R"(}})";
        }
    }

    message << R"(}})";
    return message.str();
}