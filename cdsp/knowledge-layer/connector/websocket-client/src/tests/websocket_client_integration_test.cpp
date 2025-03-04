#include <iostream>

#include "observation_id_utils.h"
#include "random_utils.h"
#include "utc_date_utils.h"
#include "websocket_client_base_integration_test.h"

class WebSocketClientIntegrationTest : public WebSocketClientBaseIntegrationTest {
   protected:
    const std::string createObservationQuery(const std::string& vin, const std::string& class_name,
                                             const std::string& observed_property,
                                             const std::string& node_value,
                                             const std::string& node_type,
                                             const std::string& start_time,
                                             const std::string& end_time) {
        return R"(
            PREFIX car: <http://example.ontology.com/car#>
            PREFIX sosa: <http://www.w3.org/ns/sosa/>
            PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>

            SELECT ?obs {
                ?obs a sosa:Observation ;
                    sosa:hasFeatureOfInterest car:)" +
               class_name + vin + R"( ;
                    sosa:hasSimpleResult ")" +
               node_value + R"("^^xsd:)" + node_type + R"( ;
                    sosa:observedProperty car:)" +
               observed_property + R"( ;
                    sosa:phenomenonTime ?time .

                FILTER (?time >= ")" +
               start_time + R"("^^xsd:dateTime && ?time <= ")" + end_time +
               R"("^^xsd:dateTime)
            } LIMIT 1
        )";
    }
};

/**
 * @brief Test case for the full flow of WebSocket client integration.
 *
 * This test simulates a message from the server and verifies that the data
 * is correctly processed and stored in RDFox. It uses a start and end time
 * to query the data and checks that the expected observation entry exists.
 */
TEST_F(WebSocketClientIntegrationTest, FullFlowTest) {
    // Set start time for the test
    const std::string start_time(UtcDateUtils::getCurrentUtcDate());

    // Simulate a message from the server
    const std::vector<MessageNodeData> nodes = {
        {"Speed", std::to_string(RandomUtils::generateRandomFloat(0, 300)), Metadata()}};
    mock_connection_->addReceivedMessage(
        createDataJsonMessage(model_config_->getObjectId()[SchemaType::VEHICLE], nodes));

    // Mock WebSocket behavior
    mockWebSocketBehavior();

    // Set end time for the test
    const std::string end_time(UtcDateUtils::getCurrentUtcDate());

    // Verify RDFox Data
    std::string sparql_query(
        createObservationQuery(model_config_->getObjectId()[SchemaType::VEHICLE], "Vehicle",
                               "speed", nodes.at(0).value, "float", start_time, end_time));

    // Assert data exists in RDFox
    std::string result = reasoner_service_->queryData(sparql_query, QueryLanguageType::SPARQL,
                                                      DataQueryAcceptType::TEXT_CSV);
    ASSERT_FALSE(result.empty())
        << "RDFox returned an empty result. Expected an observation entry.";
    ASSERT_TRUE(result.find("http://example.ontology.com/car#Observation") != std::string::npos &&
                result.find("O0") != std::string::npos)
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result << "\n";
}

/**
 * @brief Test case for the full flow of WebSocket client integration with multiple nodes.
 *
 * This test simulates two messages from the server and verifies that the data
 * is correctly processed and stored in RDFox. It uses a start and end time
 * to query the data and checks that the expected observation entries exist.
 */
TEST_F(WebSocketClientIntegrationTest, FullFlowTestMultiNodes) {
    // Set start time for the test
    const std::string start_time(UtcDateUtils::getCurrentUtcDate());

    // Simulate a first message from the server
    const std::vector<MessageNodeData> nodes_msg_1 = {
        {"Speed", std::to_string(RandomUtils::generateRandomFloat(0, 300)), Metadata()},
        {"Powertrain.TractionBattery.StateOfCharge.CurrentEnergy",
         std::to_string(RandomUtils::generateRandomFloat(0, 100)), Metadata()}};

    // Add the first simulated message to the mock connection
    mock_connection_->addReceivedMessage(
        createDataJsonMessage(model_config_->getObjectId()[SchemaType::VEHICLE], nodes_msg_1));

    // Simulate a second message from the server
    const std::vector<MessageNodeData> nodes_msg_2 = {
        {"Chassis.SteeringWheel.Angle", std::to_string(RandomUtils::generateRandomInt(0, 100)),
         Metadata()},
        {"Powertrain.TractionBattery.StateOfCharge.CurrentEnergy",
         std::to_string(RandomUtils::generateRandomFloat(0, 100)), Metadata()}};

    // Add the second simulated message to the mock connection
    mock_connection_->addReceivedMessage(
        createDataJsonMessage(model_config_->getObjectId()[SchemaType::VEHICLE], nodes_msg_2));

    // Mock WebSocket behavior
    mockWebSocketBehavior();

    // Set end time for the test
    const std::string end_time(UtcDateUtils::getCurrentUtcDate());

    // Verify RDFox Data Nodes for first message
    std::string sparql_query_msg_1_qry_1(createObservationQuery(
        model_config_->getObjectId()[SchemaType::VEHICLE], "StateOfCharge", "currentEnergy",
        nodes_msg_1.at(1).value, "float", start_time, end_time));

    std::string sparql_query_msg_1_qry_2(
        createObservationQuery(model_config_->getObjectId()[SchemaType::VEHICLE], "Vehicle",
                               "speed", nodes_msg_1.at(0).value, "float", start_time, end_time));

    // Verify RDFox Data Nodes for second message
    std::string sparql_query_msg_2_qry_1(
        createObservationQuery(model_config_->getObjectId()[SchemaType::VEHICLE], "SteeringWheel",
                               "angle", nodes_msg_2.at(0).value, "int", start_time, end_time));

    std::string sparql_query_msg_2_qry_2(createObservationQuery(
        model_config_->getObjectId()[SchemaType::VEHICLE], "StateOfCharge", "currentEnergy",
        nodes_msg_2.at(1).value, "float", start_time, end_time));

    // Assert data exists in RDFox
    std::string result_msg_1_qry_1 = reasoner_service_->queryData(
        sparql_query_msg_1_qry_1, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_CSV);
    ASSERT_FALSE(result_msg_1_qry_1.empty())
        << "RDFox returned an empty result. Expected an observation entry.";
    ASSERT_TRUE(result_msg_1_qry_1.find("http://example.ontology.com/car#Observation") !=
                    std::string::npos &&
                result_msg_1_qry_1.find("O0") != std::string::npos)
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_1_qry_1 << "\n";

    std::string result_msg_1_qry_2 = reasoner_service_->queryData(
        sparql_query_msg_1_qry_2, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_CSV);
    ASSERT_FALSE(result_msg_1_qry_2.empty())
        << "RDFox returned an empty result. Expected an observation entry.";
    ASSERT_TRUE(result_msg_1_qry_2.find("http://example.ontology.com/car#Observation") !=
                    std::string::npos &&
                result_msg_1_qry_2.find("O1") != std::string::npos)
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_1_qry_2 << "\n";

    std::string result_msg_2_qry_1 = reasoner_service_->queryData(
        sparql_query_msg_2_qry_1, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_CSV);
    ASSERT_FALSE(result_msg_2_qry_1.empty())
        << "RDFox returned an empty result. Expected an observation entry.";
    ASSERT_TRUE(result_msg_2_qry_1.find("http://example.ontology.com/car#Observation") !=
                    std::string::npos &&
                result_msg_2_qry_1.find("O0") != std::string::npos)
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_2_qry_1 << "\n";

    std::string result_msg_2_qry_2 = reasoner_service_->queryData(
        sparql_query_msg_2_qry_2, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_CSV);
    ASSERT_FALSE(result_msg_2_qry_2.empty())
        << "RDFox returned an empty result. Expected an observation entry.";
    ASSERT_TRUE(result_msg_2_qry_2.find("http://example.ontology.com/car#Observation") !=
                    std::string::npos &&
                result_msg_2_qry_2.find("O1") != std::string::npos)
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_2_qry_2 << "\n";
}