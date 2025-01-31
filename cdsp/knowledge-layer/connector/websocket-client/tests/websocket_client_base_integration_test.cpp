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

const std::string WebSocketClientBaseIntegrationTest::createUpdateMessage(
    const std::string& vin, const std::string& date_time, const std::vector<Node>& nodes) {
    std::ostringstream message;
    message << R"({
        "type": "update",
        "tree": "VSS",
        "id": ")"
            << vin << R"(",
        "dateTime": ")"
            << date_time << R"(",
        "uuid": "test",
        "nodes": [)";

    for (size_t i = 0; i < nodes.size(); ++i) {
        const auto& node = nodes[i];
        message << R"({
            "name": ")"
                << node.name << R"(",
            "value": )"
                << node.value << R"(})";

        if (i < nodes.size() - 1) {
            message << ",";
        }
    }

    message << R"(]})";
    return message.str();
}