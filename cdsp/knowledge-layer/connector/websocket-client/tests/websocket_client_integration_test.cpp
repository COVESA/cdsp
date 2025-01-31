
#include <iostream>

#include "observation_id_utils.h"
#include "random_utils.h"
#include "utc_date_utils.h"
#include "websocket_client_base_integration_test.h"

class WebSocketClientIntegrationTest : public WebSocketClientBaseIntegrationTest {
   protected:
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
               class_name + vin + R"( a car:)" + class_name + R"( .
                car:Observation)" +
               observation_id + R"( a sosa:Observation ;
                    sosa:hasFeatureOfInterest car:)" +
               class_name + vin + R"( ;
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
};

TEST_F(WebSocketClientIntegrationTest, FullFlowTest) {
    const std::string date_time(UtcDateUtils::generateRandomUtcDate());
    const std::vector<Node> nodes = {
        {"Vehicle.Speed", std::to_string(RandomUtils::generateRandomFloat(0, 300))}};

    // Simulate a message from the server
    mock_connection_->addReceivedMessage(createUpdateMessage(init_config_.oid, date_time, nodes));

    // Mock WebSocket behavior
    mockWebSocketBehavior();

    // Verify RDFox Data
    const std::string expected_observation_id(
        ObservationIdentifier::createObservationIdentifier(date_time, 0));
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
    mockWebSocketBehavior();

    // Verify RDFox Data Nodes Message 1
    const std::string exp_ob_id_msg_1_qry_1(
        ObservationIdentifier::createObservationIdentifier(date_time_msg1, 0));
    std::string sparql_query_msg_1_qry_1(
        createObservationQuery(init_config_.oid, exp_ob_id_msg_1_qry_1, "Vehicle", "speed",
                               nodes_msg_1.at(0).value, "float", date_time_msg1));

    const std::string exp_obs_id_msg_1_qry_2(
        ObservationIdentifier::createObservationIdentifier(date_time_msg1, 1));
    std::string sparql_query_msg_1_qry_2(
        createObservationQuery(init_config_.oid, exp_obs_id_msg_1_qry_2, "StateOfCharge",
                               "currentEnergy", nodes_msg_1.at(1).value, "float", date_time_msg1));

    // Verify RDFox Data Nodes Message 2
    const std::string exp_obs_id_msg_2_qry_1(
        ObservationIdentifier::createObservationIdentifier(date_time_msg2, 0));
    std::string sparql_query_msg_2_qry_1(
        createObservationQuery(init_config_.oid, exp_obs_id_msg_2_qry_1, "SteeringWheel", "angle",
                               nodes_msg_2.at(0).value, "int", date_time_msg2));

    const std::string exp_obs_id_msg_2_qry_2(
        ObservationIdentifier::createObservationIdentifier(date_time_msg2, 1));
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