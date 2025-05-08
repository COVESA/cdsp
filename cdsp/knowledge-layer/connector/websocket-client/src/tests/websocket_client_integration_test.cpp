#include <iostream>

#include "random_utils.h"
#include "utc_date_utils.h"
#include "websocket_client_base_integration_test.h"

class WebSocketClientIntegrationTest : public WebSocketClientBaseIntegrationTest {
   protected:
    const std::string createObservationQuery(const std::string& observation_id,
                                             const std::string& vin, const std::string& class_name,
                                             const std::string& observed_property,
                                             const std::string& node_value,
                                             const std::string& node_type,
                                             const std::string& phenomenon_time) {
        return R"(
            PREFIX car: <http://example.ontology.com/car#>
            PREFIX sosa: <http://www.w3.org/ns/sosa/>
            PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>

            ASK {
                car:ob_)" +
               observed_property + "_" + observation_id + R"( a sosa:Observation ;
                    sosa:hasFeatureOfInterest car:)" +
               class_name + vin + R"( ;
                    sosa:hasSimpleResult ")" +
               node_value + R"("^^xsd:)" + node_type + R"( ;
                    sosa:observedProperty car:)" +
               observed_property + R"( ;
                    sosa:phenomenonTime ")" +
               phenomenon_time + R"("^^xsd:dateTime .
                }
            )";
    };
};

/**
 * @brief Test case for the full flow of WebSocket client integration.
 *
 * This test simulates a message from the server and verifies that the data
 * is correctly processed and stored in RDFox. It uses a start and end time
 * to query the data and checks that the expected observation entry exists.
 */
TEST_F(WebSocketClientIntegrationTest, FullFlowTest) {
    // Simulate a message from the server
    const Metadata metadata = Metadata(RandomUtils::generateRandomTimestamp(2020, 2030, true),
                                       RandomUtils::generateRandomTimestamp(2020, 2030, true));
    const std::vector<MessageNodeData> nodes = {
        {"Speed", std::to_string(RandomUtils::generateRandomFloat(0, 300)), metadata}};
    mock_connection_->addReceivedMessage(
        createDataJsonMessage(model_config_->getObjectId()[SchemaType::VEHICLE], nodes));

    // Mock WebSocket behavior
    mockWebSocketBehavior();

    // Verify RDFox Data
    std::string timestamp =
        Helper::getFormattedTimestampCustom("%Y%m%d%H%M%S", metadata.getGenerated().value());

    std::string phenomenon_time = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", metadata.getGenerated().value(), true);

    std::string observation_identifier =
        timestamp + Helper::extractNanoseconds(metadata.getGenerated().value());

    std::string sparql_query(createObservationQuery(
        observation_identifier, model_config_->getObjectId()[SchemaType::VEHICLE], "Vehicle",
        "speed", nodes.at(0).value, "float", phenomenon_time));

    // Assert data exists in RDFox
    std::string result = reasoner_service_->queryData(sparql_query, QueryLanguageType::SPARQL,
                                                      DataQueryAcceptType::SPARQL_JSON);

    // Parse the JSON result to check if the observation exists
    nlohmann::json json_result = nlohmann::json::parse(result);
    ASSERT_TRUE(json_result.contains("boolean"))
        << "RDFox returned an empty result. Expected an observation entry.";

    ASSERT_TRUE(json_result["boolean"]) << "Observation was not found in RDFox!\n"
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
    // Simulate a first message from the server
    const Metadata metadata_msg_1 =
        Metadata(RandomUtils::generateRandomTimestamp(2020, 2030, true),
                 RandomUtils::generateRandomTimestamp(2020, 2030, true));
    const std::vector<MessageNodeData> nodes_msg_1 = {
        {"Speed", std::to_string(RandomUtils::generateRandomFloat(0, 300)), metadata_msg_1},
        {"Powertrain.TractionBattery.StateOfCharge.CurrentEnergy",
         std::to_string(RandomUtils::generateRandomFloat(0, 100)), metadata_msg_1}};

    // Add the first simulated message to the mock connection
    mock_connection_->addReceivedMessage(
        createDataJsonMessage(model_config_->getObjectId()[SchemaType::VEHICLE], nodes_msg_1));

    // Simulate a second message from the server
    const Metadata metadata_msg_2 =
        Metadata(RandomUtils::generateRandomTimestamp(2020, 2030, true),
                 RandomUtils::generateRandomTimestamp(2020, 2030, true));
    const std::vector<MessageNodeData> nodes_msg_2 = {
        {"Chassis.SteeringWheel.Angle", std::to_string(RandomUtils::generateRandomInt(0, 100)),
         metadata_msg_2},
        {"Powertrain.TractionBattery.StateOfCharge.CurrentEnergy",
         std::to_string(RandomUtils::generateRandomFloat(0, 100)), metadata_msg_2}};

    // Add the second simulated message to the mock connection
    mock_connection_->addReceivedMessage(
        createDataJsonMessage(model_config_->getObjectId()[SchemaType::VEHICLE], nodes_msg_2));

    // Mock WebSocket behavior
    mockWebSocketBehavior();

    // ** Verify RDFox Data Nodes for first message **

    std::string timestamp_msg_1 =
        Helper::getFormattedTimestampCustom("%Y%m%d%H%M%S", metadata_msg_1.getGenerated().value());

    std::string phenomenon_time_msg_1 = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", metadata_msg_1.getGenerated().value(), true);

    std::string observation_identifier_msg_1 =
        timestamp_msg_1 + Helper::extractNanoseconds(metadata_msg_1.getGenerated().value());

    std::string sparql_query_msg_1_qry_1(createObservationQuery(
        observation_identifier_msg_1, model_config_->getObjectId()[SchemaType::VEHICLE],
        "StateOfCharge", "currentEnergy", nodes_msg_1.at(1).value, "float", phenomenon_time_msg_1));

    std::string sparql_query_msg_1_qry_2(createObservationQuery(
        observation_identifier_msg_1, model_config_->getObjectId()[SchemaType::VEHICLE], "Vehicle",
        "speed", nodes_msg_1.at(0).value, "float", phenomenon_time_msg_1));

    // ** Verify RDFox Data Nodes for second message **

    std::string timestamp_msg_2 =
        Helper::getFormattedTimestampCustom("%Y%m%d%H%M%S", metadata_msg_2.getGenerated().value());

    std::string phenomenon_time_msg_2 = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", metadata_msg_2.getGenerated().value(), true);

    std::string observation_identifier_msg_2 =
        timestamp_msg_2 + Helper::extractNanoseconds(metadata_msg_2.getGenerated().value());

    std::string sparql_query_msg_2_qry_1(createObservationQuery(
        observation_identifier_msg_2, model_config_->getObjectId()[SchemaType::VEHICLE],
        "SteeringWheel", "angle", nodes_msg_2.at(0).value, "int", phenomenon_time_msg_2));

    std::string sparql_query_msg_2_qry_2(createObservationQuery(
        observation_identifier_msg_2, model_config_->getObjectId()[SchemaType::VEHICLE],
        "StateOfCharge", "currentEnergy", nodes_msg_2.at(1).value, "float", phenomenon_time_msg_2));

    // Assert data exists in RDFox

    // ** Assert data exists in RDFox for first message and first node **
    std::string result_msg_1_qry_1 = reasoner_service_->queryData(
        sparql_query_msg_1_qry_1, QueryLanguageType::SPARQL, DataQueryAcceptType::SPARQL_JSON);

    // Parse the JSON result to check if the observation exists for the first message and first node
    nlohmann::json json_result_msg_1_qry_1 = nlohmann::json::parse(result_msg_1_qry_1);
    ASSERT_TRUE(json_result_msg_1_qry_1.contains("boolean"))
        << "RDFox returned an empty result. Expected an observation entry.";

    ASSERT_TRUE(json_result_msg_1_qry_1["boolean"])
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_1_qry_1 << "\n";

    // ** Assert data exists in RDFox for first message and second node **
    std::string result_msg_1_qry_2 = reasoner_service_->queryData(
        sparql_query_msg_1_qry_2, QueryLanguageType::SPARQL, DataQueryAcceptType::SPARQL_JSON);

    // Parse the JSON result to check if the observation exists for the first message and second
    // node
    nlohmann::json json_result_msg_1_qry_2 = nlohmann::json::parse(result_msg_1_qry_2);
    ASSERT_TRUE(json_result_msg_1_qry_2.contains("boolean"))
        << "RDFox returned an empty result. Expected an observation entry.";

    ASSERT_TRUE(json_result_msg_1_qry_2["boolean"])
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_1_qry_2 << "\n";

    // ** Assert data exists in RDFox for second message and first node **
    std::string result_msg_2_qry_1 = reasoner_service_->queryData(
        sparql_query_msg_2_qry_1, QueryLanguageType::SPARQL, DataQueryAcceptType::SPARQL_JSON);

    // Parse the JSON result to check if the observation exists for the second message and first
    // node
    nlohmann::json json_result_msg_2_qry_1 = nlohmann::json::parse(result_msg_2_qry_1);
    ASSERT_TRUE(json_result_msg_2_qry_1.contains("boolean"))
        << "RDFox returned an empty result. Expected an observation entry.";

    ASSERT_TRUE(json_result_msg_2_qry_1["boolean"])
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_2_qry_1 << "\n";

    // ** Assert data exists in RDFox for second message and second node **
    std::string result_msg_2_qry_2 = reasoner_service_->queryData(
        sparql_query_msg_2_qry_2, QueryLanguageType::SPARQL, DataQueryAcceptType::SPARQL_JSON);

    // Parse the JSON result to check if the observation exists for the second message and second
    // node
    nlohmann::json json_result_msg_2_qry_2 = nlohmann::json::parse(result_msg_2_qry_2);
    ASSERT_TRUE(json_result_msg_2_qry_2.contains("boolean"))
        << "RDFox returned an empty result. Expected an observation entry.";
    ASSERT_TRUE(json_result_msg_2_qry_2["boolean"])
        << "Observation was not found in RDFox!\n"
        << "Query Result: " << result_msg_2_qry_2 << "\n";
}