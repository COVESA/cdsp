// WebSocketClientBaseIntegrationTest.h
#ifndef WEBSOCKET_CLIENT_BASE_INTEGRATION_TEST_H
#define WEBSOCKET_CLIENT_BASE_INTEGRATION_TEST_H

#include <gtest/gtest.h>

#include <memory>

#include "file_handler_impl.h"
#include "globals.h"
#include "mock_websocket_connection.h"
#include "reasoner_service.h"
#include "triple_assembler.h"
#include "triple_writer.h"
#include "websocket_client.h"

class WebSocketClientBaseIntegrationTest : public ::testing::Test {
   protected:
    const std::string MODEL_CONFIGURATION_FILE =
        getProjectRoot() + "/symbolic-reasoner/examples/use-case/model/model_config.json";
    struct MessageNodeData {
        std::string name;
        std::string value;
        Metadata metadata;
    };

    void SetUp() override;
    void TearDown() override;
    void mockWebSocketBehavior();
    const std::string createDataJsonMessage(const std::string& vin,
                                            const std::vector<MessageNodeData>& nodes);
    SystemConfig system_config_;
    std::shared_ptr<MockWebSocketConnection> mock_connection_;
    std::shared_ptr<WebSocketClient> client_;
    std::shared_ptr<ReasonerService> reasoner_service_;
    std::shared_ptr<ModelConfig> model_config_;
};

#endif  // WEBSOCKET_CLIENT_BASE_INTEGRATION_TEST_H