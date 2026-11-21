#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <string>

#include "mock_reasoner_adapter.h"
#include "mock_reasoner_service.h"
#include "websocket_client.h"
#include "websocket_interface.h"

using ::testing::_;
using ::testing::Return;

class MockConnection : public WebSocketClientInterface {
   public:
    MOCK_METHOD(void, asyncResolve, (const std::string& host, const std::string& port), (override));
    MOCK_METHOD(void, asyncConnect, (), (override));
    MOCK_METHOD(void, asyncHandshake, (), (override));
    MOCK_METHOD(void, asyncWrite, (const json& message), (override));
    MOCK_METHOD(void, asyncRead, (), (override));
    MOCK_METHOD(std::string, getReceivedMessage, (), (override));
    MOCK_METHOD(void, consumeBuffer, (std::size_t bytes), (override));
};

/**
 * A message that cannot be processed must be skipped: the client keeps running and reads the next
 * message instead of ending the process.
 */
class WebSocketClientUnitTest : public ::testing::Test {
   protected:
    std::shared_ptr<MockConnection> connection_ = std::make_shared<MockConnection>();
    std::shared_ptr<MockReasonerService> reasoner_service_;
    std::shared_ptr<WebSocketClient> client_;

    void SetUp() override {
        setenv("VEHICLE_OBJECT_ID", "VINABCD1234567890", 1);
        auto adapter = std::make_shared<MockReasonerAdapter>();
        reasoner_service_ = std::make_shared<MockReasonerService>(adapter);
        ON_CALL(*reasoner_service_, checkDataStore()).WillByDefault(Return(true));
        ON_CALL(*reasoner_service_, loadData(_, _)).WillByDefault(Return(true));

        auto model_config = std::make_shared<ModelConfig>(
            std::map<SchemaType, SchemaInputList>{{SchemaType::VEHICLE, SchemaInputList{{"Speed"}}}},
            std::vector<std::pair<ReasonerSyntaxType, std::string>>{}, "output/",
            std::vector<std::pair<RuleLanguageType, std::string>>{{RuleLanguageType::DATALOG, "rule"}},
            std::vector<std::pair<ReasonerSyntaxType, std::string>>{
                {ReasonerSyntaxType::TURTLE, "shapes"}},
            TripleAssemblerHelper({{SchemaType::VEHICLE,
                                    {{QueryLanguageType::SPARQL, "data_property"},
                                     {QueryLanguageType::SPARQL, "object_property"}}}}),
            std::vector<ReasoningOutputQuery>{{QueryLanguageType::SPARQL, "query"}},
            ReasonerSettings(InferenceEngineType::RDF4J, ReasonerSyntaxType::TURTLE,
                             {SchemaType::VEHICLE}, true));

        client_ = std::make_shared<WebSocketClient>(SystemConfig{}, model_config, reasoner_service_,
                                                    connection_);
    }

    void receive(const std::string& message) {
        EXPECT_CALL(*connection_, getReceivedMessage()).WillOnce(Return(message));
        EXPECT_CALL(*connection_, consumeBuffer(_));
        client_->onReceiveMessage({}, message.size());
    }
};

TEST_F(WebSocketClientUnitTest, SkipsInvalidJsonAndReadsNextMessage) {
    EXPECT_CALL(*connection_, asyncRead());

    EXPECT_NO_THROW(receive("this is not JSON"));
}

TEST_F(WebSocketClientUnitTest, SkipsDataMessageWhenDatastoreIsNotReachable) {
    // The handshake queues a get (request id 0) and a subscribe (request id 1) and sends the get.
    // After the data message for the get is skipped, the client continues with the queued subscribe.
    EXPECT_CALL(*connection_, asyncWrite(_)).Times(2);
    client_->handshake({});

    EXPECT_CALL(*reasoner_service_, checkDataStore()).WillOnce(Return(false));
    EXPECT_CALL(*reasoner_service_, queryData(_, _, _)).Times(0);

    EXPECT_NO_THROW(receive(
        R"({"jsonrpc":"2.0","id":0,"result":{"data":{"":60},"instance":"VINABCD1234567890",)"
        R"("metadata":{"":{"timestamps":{"received":{"seconds":1,"nanos":0}}}}}})"));
}
