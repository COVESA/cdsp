#include <gtest/gtest.h>

#include <iostream>

#include "file_handler_impl.h"
#include "init_config_fixture.h"
#include "mock_websocket_connection.h"
#include "model_config.h"
#include "random_utils.h"
#include "rdfox_adapter.h"
#include "triple_assembler.h"
#include "triple_writer.h"
#include "utc_date_utils.h"
#include "vin_utils.h"
#include "websocket_client.h"

class WebSocketClientIntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // get a random object id (VIN)
        const std::string vin(VinUtils::getRandomVinString());

        ModelConfig model_config;
        loadModelConfig(std::string(PROJECT_ROOT) +
                            "/symbolic-reasoner/examples/use-case/model/model_config.json",
                        model_config);

        init_config_ = InitConfigFixture::getValidInitConfig(vin);
        init_config_.model_config = model_config;

        // Initialize RDFoxAdapter
        rdfox_adapter_ = std::make_shared<RDFoxAdapter>(
            init_config_.rdfox_server.host, init_config_.rdfox_server.port,
            init_config_.rdfox_server.auth_base64, init_config_.rdfox_server.data_store.value());
        rdfox_adapter_->initialize();

        // Initialize TripleAssembler
        triple_assembler_ = std::make_shared<TripleAssembler>(
            init_config_.model_config, *rdfox_adapter_, file_handler_, triple_writer_);
        triple_assembler_->initialize();

        // Initialize Mock WebSocket connection
        mock_connection_ = std::make_shared<MockWebSocketConnection>();

        // Create WebSocketClient with mock WebSocket connection
        client_ =
            std::make_shared<WebSocketClient>(init_config_, *triple_assembler_, mock_connection_);
    }

    void TearDown() override {  // Delete the test data store after all tests have completed
        ASSERT_TRUE(rdfox_adapter_->deleteDataStore()) << "Failed to clean up the test data store.";
    }

    const std::string createObservationId(const std::string& date_time) {
        const auto [dateTime, _] = Helper::parseISO8601ToTime(date_time);
        return Helper::getFormattedTimestamp("%Y%m%d%H%M%S", false, false, dateTime);
    }

    const std::string createObservationQuery(
        const std::string& vin, const std::string& observation_id, const std::string& class_name,
        const std::string& observed_property, const std::string& node_value,
        const std::string& node_type, const std::string& date_time) {
        return R"(
        PREFIX car: <http://example.ontology.com/car#>
        PREFIX sosa: <http://www.w3.org/ns/sosa/>
        PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>

        SELECT ?exists WHERE {
            {
                car:)" +
               class_name + R"()" + vin + R"( a car:)" + class_name + R"( .
                car:Observation)" +
               observation_id + R"( a sosa:Observation ;
                    sosa:hasFeatureOfInterest car:)" +
               class_name + R"()" + vin + R"( ;
                    sosa:hasSimpleResult ")" +
               node_value + R"("^^xsd:)" + node_type + R"( ;
                    sosa:observedProperty car:)" +
               observed_property + R"( ;
                    sosa:phenomenonTime ")" +
               date_time + R"("^^xsd:dateTime .
            }
            BIND(true AS ?exists)
        } LIMIT 1
    )";
    }

    const std::string createUpdateMessage(const std::string& vin, const std::string& date_time,
                                          const std::vector<Node>& nodes) {
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

    InitConfig init_config_;
    std::shared_ptr<RDFoxAdapter> rdfox_adapter_;
    std::shared_ptr<TripleAssembler> triple_assembler_;
    FileHandlerImpl file_handler_;
    TripleWriter triple_writer_;
    std::shared_ptr<MockWebSocketConnection> mock_connection_;
    std::shared_ptr<WebSocketClient> client_;
};

TEST_F(WebSocketClientIntegrationTest, FullFlowTest) {
    const std::string date_time(UtcDateUtils::generateRandomUtcDate());
    const std::vector<Node> nodes = {
        {"Vehicle.Speed", std::to_string(RandomUtils::generateRandomFloat(0, 300))}};

    // Simulate a message from the server
    mock_connection_->addReceivedMessage(createUpdateMessage(init_config_.oid, date_time, nodes));

    // Mock WebSocket behavior
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

    // Run the WebSocket client
    client_->run();

    // Verify RDFox Data
    const std::string expected_observation_id(createObservationId(date_time) + "O0");
    std::string sparql_query(createObservationQuery(init_config_.oid, expected_observation_id,
                                                    "Vehicle", "speed", nodes.at(0).value, "float",
                                                    date_time));

    std::string result = rdfox_adapter_->queryData(sparql_query);

    // Assert data exists in RDFox
    ASSERT_TRUE(result.find("true") != std::string::npos) << "Triple was not found in RDFox!";
}

TEST_F(WebSocketClientIntegrationTest, FullFlowTestMultiNodes) {
    // Data for update message 1
    const std::string date_time_msg1(UtcDateUtils::generateRandomUtcDate());
    const std::vector<Node> nodes_msg_1 = {
        {"Vehicle.Speed", std::to_string(RandomUtils::generateRandomFloat(0, 300))},
        {"Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy",
         std::to_string(RandomUtils::generateRandomFloat(0, 100))}};

    // Data for update message 2
    const std::string date_time_msg2(UtcDateUtils::generateRandomUtcDate());
    const std::string observation_id_msg_2(createObservationId(date_time_msg2));
    const std::vector<Node> nodes_msg_2 = {
        {"Vehicle.Chassis.SteeringWheel.Angle",
         std::to_string(RandomUtils::generateRandomInt(0, 100))},
        {"Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy",
         std::to_string(RandomUtils::generateRandomFloat(0, 100))}};

    // Simulate a message from the server
    mock_connection_->addReceivedMessage(
        createUpdateMessage(init_config_.oid, date_time_msg1, nodes_msg_1));
    mock_connection_->addReceivedMessage(
        createUpdateMessage(init_config_.oid, date_time_msg2, nodes_msg_2));

    // Mock WebSocket behavior
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

    // Run the WebSocket client
    client_->run();

    // Verify RDFox Data Nodes Message 1
    const std::string exp_ob_id_msg_1_qry_1(createObservationId(date_time_msg1) + "O0");
    std::string sparql_query_msg_1_qry_1(
        createObservationQuery(init_config_.oid, exp_ob_id_msg_1_qry_1, "Vehicle", "speed",
                               nodes_msg_1.at(0).value, "float", date_time_msg1));

    const std::string exp_obs_id_msg_1_qry_2(createObservationId(date_time_msg1) + "O1");
    std::string sparql_query_msg_1_qry_2(
        createObservationQuery(init_config_.oid, exp_obs_id_msg_1_qry_2, "StateOfCharge",
                               "currentEnergy", nodes_msg_1.at(1).value, "float", date_time_msg1));

    // Verify RDFox Data Nodes Message 2
    const std::string exp_obs_id_msg_2_qry_1(createObservationId(date_time_msg2) + "O0");
    std::string sparql_query_msg_2_qry_1(
        createObservationQuery(init_config_.oid, exp_obs_id_msg_2_qry_1, "SteeringWheel", "angle",
                               nodes_msg_2.at(0).value, "int", date_time_msg2));

    const std::string exp_obs_id_msg_2_qry_2(createObservationId(date_time_msg2) + "O1");
    std::string sparql_query_msg_2_qry_2(
        createObservationQuery(init_config_.oid, exp_obs_id_msg_2_qry_2, "StateOfCharge",
                               "currentEnergy", nodes_msg_2.at(1).value, "float", date_time_msg2));

    std::string result_msg_1_qry_1 = rdfox_adapter_->queryData(sparql_query_msg_1_qry_1);
    std::string result_msg_1_qry_2 = rdfox_adapter_->queryData(sparql_query_msg_1_qry_2);
    std::string result_msg_2_qry_1 = rdfox_adapter_->queryData(sparql_query_msg_2_qry_1);
    std::string result_msg_2_qry_2 = rdfox_adapter_->queryData(sparql_query_msg_2_qry_2);

    // Assert data exists in RDFox
    ASSERT_TRUE(result_msg_1_qry_1.find("true") != std::string::npos)
        << "Triple for message 1, node 1 was not found in RDFox!";
    ASSERT_TRUE(result_msg_1_qry_2.find("true") != std::string::npos)
        << "Triple for message 1, node 2 was not found in RDFox!";
    ASSERT_TRUE(result_msg_2_qry_1.find("true") != std::string::npos)
        << "Triple for message 2, node 1 was not found in RDFox!";
    ASSERT_TRUE(result_msg_2_qry_2.find("true") != std::string::npos)
        << "Triple for message 2, node 2 was not found in RDFox!";
}