#include "websocket_client_base_integration_test.h"

#include <cstdlib>
#include <iostream>

#include "rdfox_adapter.h"
#include "reasoner_factory.h"
#include "server_data_fixture.h"
#include "system_configuration_service.h"
#include "vin_utils.h"

void WebSocketClientBaseIntegrationTest::SetUp() {
    // ** Initialize Main Services **
    setenv("VEHICLE_OBJECT_ID", VinUtils::getRandomVinString().c_str(), 1);

    // Initialize System Configuration
    SystemConfig system_config;
    system_config.reasoner_server = ServerDataFixture::getValidRDFoxServerData();

    // Initialize Model Configuration
    model_config_ = std::make_shared<ModelConfig>(
        SystemConfigurationService::loadModelConfig(MODEL_CONFIGURATION_FILE));

    // Initialize Reasoner Service
    reasoner_service_ = ReasonerFactory::initReasoner(
        model_config_->getReasonerSettings().getInferenceEngine(), system_config.reasoner_server,
        model_config_->getReasonerRules(), model_config_->getOntologies(), false);
    if (!reasoner_service_) {
        throw std::runtime_error("Failed to initialize the reasoner service.");
    }

    // Initialize WebSocket Client
    mock_connection_ = std::make_shared<MockWebSocketConnection>();

    client_ = std::make_shared<WebSocketClient>(system_config, model_config_, reasoner_service_,
                                                mock_connection_);
}

void WebSocketClientBaseIntegrationTest::TearDown() {
    ASSERT_TRUE(reasoner_service_->deleteDataStore()) << "Failed to clean up the test data store.";
}

/**
 * @brief Mocks the behavior of a WebSocket connection for integration testing.
 *
 * This function sets up mock callbacks for various stages of a WebSocket connection,
 * including resolution, connection, handshake, and reading messages. Each callback
 * logs a message to the console and triggers the next step in the WebSocket process.
 * Finally, it runs the WebSocket client to simulate the connection behavior.
 */
void WebSocketClientBaseIntegrationTest::mockWebSocketBehavior() {
    mock_connection_->setOnResolveCallback([&]() {
        std::cout << "Mock: Resolved WebSocket Server." << std::endl;
        mock_connection_->asyncConnect();
    });

    mock_connection_->setOnConnectCallback([&]() {
        std::cout << "Mock: Connected to Websocket server." << std::endl;
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
                "nanos": )"
                        << std::stoi(Helper::extractNanoseconds(generated_time)) << R"(})";
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
                "nanos": )"
                        << std::stoi(Helper::extractNanoseconds(received_time)) << R"(})";
            }

            message << R"(}})";
        }
    }

    message << R"(}})";
    return message.str();
}