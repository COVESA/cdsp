// WebSocketClientBaseIntegrationTest.h
#ifndef WEBSOCKET_CLIENT_BASE_INTEGRATION_TEST_H
#define WEBSOCKET_CLIENT_BASE_INTEGRATION_TEST_H

#include <gtest/gtest.h>

#include <memory>

#include "file_handler_impl.h"
#include "init_config_fixture.h"
#include "mock_websocket_connection.h"
#include "model_config_utils.h"
#include "rdfox_adapter.h"
#include "triple_assembler.h"
#include "triple_writer.h"
#include "websocket_client.h"

class WebSocketClientBaseIntegrationTest : public ::testing::Test {
   protected:
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

    InitConfig init_config_;
    std::shared_ptr<RDFoxAdapter> rdfox_adapter_;
    std::shared_ptr<TripleAssembler> triple_assembler_;
    FileHandlerImpl file_handler_;
    TripleWriter triple_writer_;
    std::shared_ptr<MockWebSocketConnection> mock_connection_;
    std::shared_ptr<WebSocketClient> client_;
};

#endif  // WEBSOCKET_CLIENT_BASE_INTEGRATION_TEST_H