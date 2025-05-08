#include <iostream>
#include <string>

#include "helper.h"
#include "random_utils.h"
#include "utc_date_utils.h"
#include "websocket_client_base_integration_test.h"

#define ASSERT_CONDITIONAL(condition, expected, message) \
    do {                                                 \
        if (expected) {                                  \
            ASSERT_TRUE(condition) << message;           \
        } else {                                         \
            ASSERT_FALSE(condition) << message;          \
        }                                                \
    } while (0)

class WebSocketClientCoordinatesIntegrationTest : public WebSocketClientBaseIntegrationTest {
   protected:
    const std::string extractNumericTimestamp(const std::string& timestamp) {
        std::string compact_time;
        for (char c : timestamp) {
            if (std::isdigit(c)) {
                compact_time += c;
            }
        }
        return compact_time;
    }
    const std::string createObservationQuery(const std::string& vin, const std::string& class_name,
                                             const std::string& observed_property,
                                             const std::string& node_value,
                                             const std::string& node_type,
                                             const std::string& data_time) {
        return R"(
            PREFIX car: <http://example.ontology.com/car#>
            PREFIX sosa: <http://www.w3.org/ns/sosa/>
            PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>

            SELECT ?obs ?ntmValue {
                    ?obs a sosa:Observation ;
                        sosa:hasFeatureOfInterest car:)" +
               class_name + vin + R"( ;
                        sosa:hasSimpleResult ")" +
               node_value + R"("^^xsd:)" + node_type + R"( ;
                        sosa:observedProperty car:)" +
               observed_property + R"( ;
                        sosa:phenomenonTime ")" +
               data_time + R"("^^xsd:dateTime ;
                        car:hasSimpleResultNTM ?ntmValue.                
            } LIMIT 1
        )";
    }

    /**
     * Processes and verifies messages by adding them to a mock connection, executing a mock
     * WebSocket behavior, and querying RDFox to verify the existence of latitude and longitude
     * observations.
     *
     * @param messages_nodes A vector of vectors containing Node objects representing the messages
     * to be processed.
     * @param latitude The expected latitude value to verify in the RDFox query.
     * @param longitude The expected longitude value to verify in the RDFox query.
     * @param expect_exists A boolean indicating whether the observations are expected to exist in
     * the RDFox query results.
     * @param consecutive_observation_latitude An integer representing the expected consecutive
     * observation ID for latitude. Defaults to 0.
     * @param consecutive_observation_longitude An integer representing the expected consecutive
     * observation ID for longitude. Defaults to 1.
     */
    void processAndVerifyMessages(const std::vector<std::vector<MessageNodeData>>& messages_nodes,
                                  const std::string& latitude, const std::string& longitude,
                                  const std::string& data_time_latitude,
                                  const std::string& data_time_longitude, bool expect_exists) {
        // Add received messages_nodes
        for (const auto& nodes : messages_nodes) {
            std::string vin = model_config_->getObjectId()[SchemaType::VEHICLE];
            mock_connection_->addReceivedMessage(createDataJsonMessage(vin, nodes));
        }

        // Use the inherited mockWebSocketBehavior
        mockWebSocketBehavior();

        // Verify triples in RDFox
        std::string vin = model_config_->getObjectId()[SchemaType::VEHICLE];
        std::string sparql_query_latitude = createObservationQuery(
            vin, "CurrentLocation", "latitude", latitude, "double", data_time_latitude);
        std::string sparql_query_longitude = createObservationQuery(
            vin, "CurrentLocation", "longitude", longitude, "double", data_time_longitude);

        std::string result_latitude = reasoner_service_->queryData(
            sparql_query_latitude, QueryLanguageType::SPARQL, DataQueryAcceptType::SPARQL_JSON);
        std::string result_longitude = reasoner_service_->queryData(
            sparql_query_longitude, QueryLanguageType::SPARQL, DataQueryAcceptType::SPARQL_JSON);

        // Helper function to check if the observation exists and `ntmValue` is valid
        auto verifyObservation = [](const std::string& result,
                                    const std::string& expected_observation_id, bool expect_exist) {
            try {
                json json_result = json::parse(result);

                // Check if the expected structure exists
                if (!json_result.contains("results") ||
                    !json_result["results"].contains("bindings")) {
                    FAIL() << "Missing expected observation data in RDFox response!";
                }
                if (!expect_exist) {
                    ASSERT_TRUE(json_result["results"]["bindings"].empty())
                        << "Unexpected observation data found in RDFox response!";
                } else {
                    json first_binding = json_result["results"]["bindings"][0];

                    // Assert observation ID exists
                    ASSERT_TRUE(first_binding.contains("obs") &&
                                first_binding["obs"].contains("value"))
                        << "Observation ID is missing in RDFox response!";
                    std::string observation_id = first_binding["obs"]["value"];
                    ASSERT_TRUE(observation_id.find(expected_observation_id) != std::string::npos)
                        << "Expected observation ID `" << expected_observation_id
                        << "` not found! Found: " << observation_id;

                    // Assert `ntmValue` exists and is a valid float/double
                    ASSERT_TRUE(first_binding.contains("ntmValue") &&
                                first_binding["ntmValue"].contains("value"))
                        << "ntmValue is missing in RDFox response!";
                    std::string ntm_value_str = first_binding["ntmValue"]["value"];

                    std::regex float_regex(R"([-+]?\d*\.?\d+([eE][-+]?\d+)?)");
                    ASSERT_TRUE(std::regex_match(ntm_value_str, float_regex))
                        << "Invalid ntmValue format: " << ntm_value_str;
                }

            } catch (const std::exception& e) {
                FAIL() << "JSON Parsing failed: " << e.what();
            }
        };
        // Check latitude and longitude observations
        verifyObservation(result_latitude, extractNumericTimestamp(data_time_latitude),
                          expect_exists);
        verifyObservation(result_longitude, extractNumericTimestamp(data_time_longitude),
                          expect_exists);
    }
};

/**
 * @brief Test case for verifying that coordinates in the same message generate triples.
 *
 * This test generates random latitude and longitude values, constructs message nodes
 * with these coordinates, and verifies that the messages are processed correctly to
 * generate triples.
 */
TEST_F(WebSocketClientCoordinatesIntegrationTest, CoordinatesInSameMessageGenerateTriples) {
    // Generate random latitude and longitude values as strings
    const std::string latitude = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    // Create message nodes with latitude and longitude data
    const std::vector<MessageNodeData> nodes = {
        {"CurrentLocation.Latitude", latitude, Metadata()},
        {"CurrentLocation.Longitude", longitude, Metadata()}};

    // Format the timestamps for latitude and longitude data
    std::string data_time_latitude = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", nodes.at(0).metadata.getReceived(), true);
    std::string data_time_longitude = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", nodes.at(1).metadata.getReceived(), true);

    // Process and verify the messages with the generated coordinates and timestamps
    processAndVerifyMessages({nodes}, latitude, longitude, data_time_latitude, data_time_longitude,
                             true);
}

/**
 * @brief Test to verify that coordinates received in different messages within two seconds
 * generate triples.
 *
 * This test simulates the reception of coordinates in separate messages with specific timestamps
 * and verifies that they are processed correctly to form triples. The timestamps are generated
 * such that some messages are within the two-second window required to form a triple, while others
 * are not.
 */
TEST_F(WebSocketClientCoordinatesIntegrationTest,
       CoordinatesInDifferentMessagesWithinTwoSecondsGenerateTriples) {
    // Generate timestamps for each message
    std::chrono::system_clock::time_point timestamp_msg1 = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point timestamp_msg2 = timestamp_msg1 + std::chrono::seconds(3);
    std::chrono::system_clock::time_point timestamp_msg3 = timestamp_msg2 + std::chrono::seconds(1);
    std::chrono::system_clock::time_point timestamp_msg4 =
        timestamp_msg3 + std::chrono::milliseconds(1500);

    // Create Metadata for each message using generated timestamps
    Metadata metadata_msg1 = Metadata(std::nullopt, timestamp_msg1);
    Metadata metadata_msg2 = Metadata(std::nullopt, timestamp_msg2);
    Metadata metadata_msg3 = Metadata(std::nullopt, timestamp_msg3);
    Metadata metadata_msg4 = Metadata(std::nullopt, timestamp_msg4);

    // Generate random coordinates
    const std::string latitude_msg1 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude_msg2 = std::to_string(RandomUtils::generateRandomDouble(-63, 86));
    const std::string longitude_msg3 = std::to_string(RandomUtils::generateRandomDouble(-63, 86));
    const std::string latitude_msg4 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));

    const std::vector<MessageNodeData> node_msg_1 = {
        {"CurrentLocation.Latitude", latitude_msg1, metadata_msg1}};
    const std::vector<MessageNodeData> node_msg_2 = {
        {"CurrentLocation.Longitude", longitude_msg2, metadata_msg2}};
    const std::vector<MessageNodeData> node_msg_3 = {
        {"CurrentLocation.Longitude", longitude_msg3, metadata_msg3}};
    const std::vector<MessageNodeData> node_msg_4 = {
        {"CurrentLocation.Latitude", latitude_msg4, metadata_msg4}};

    std::string data_time_longitude_msg3 = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", node_msg_3.at(0).metadata.getGenerated().value(), true);
    std::string data_time_latitude_msg4 = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", node_msg_4.at(0).metadata.getGenerated().value(), true);

    // Process and verify the messages with the generated coordinates and timestamps
    processAndVerifyMessages({node_msg_1, node_msg_2, node_msg_3, node_msg_4}, latitude_msg4,
                             longitude_msg3, data_time_latitude_msg4, data_time_longitude_msg3,
                             true);
}

/**
 * @brief Test case for verifying that coordinates in different messages after a two-second interval
 * generate any triples.
 *
 * This test simulates the reception of coordinates in separate messages with specific timestamps
 * and verifies that they are processed correctly to form triples. The timestamps are generated such
 * that the messages are not within the two-second window required to form a triple.
 */
TEST_F(WebSocketClientCoordinatesIntegrationTest,
       CoordinatesInDifferentMessagesAfterTwoSecondsGenerateAnyTriples) {
    // Generate timestamps for each message, with a 3-second interval between them
    std::chrono::system_clock::time_point timestamp_msg1 = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point timestamp_msg2 = timestamp_msg1 + std::chrono::seconds(3);

    // Create Metadata objects for each message using the generated timestamps
    Metadata metadata_msg1 = Metadata(std::nullopt, timestamp_msg1);
    Metadata metadata_msg2 = Metadata(std::nullopt, timestamp_msg2);

    // Generate random latitude and longitude coordinates as strings
    const std::string latitude = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    // Create message node data for latitude and longitude with associated metadata
    const std::vector<MessageNodeData> node_msg_1 = {
        {"CurrentLocation.Latitude", latitude, metadata_msg1}};
    const std::vector<MessageNodeData> node_msg_2 = {
        {"CurrentLocation.Longitude", longitude, metadata_msg2}};

    // Format the timestamps for latitude and longitude messages into a custom string format
    std::string data_time_latitude_msg1 = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", node_msg_1.at(0).metadata.getGenerated().value(), true);
    std::string data_time_longitude_msg2 = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", node_msg_2.at(0).metadata.getGenerated().value(), true);

    // Process and verify the messages with the generated coordinates and timestamps
    processAndVerifyMessages({node_msg_1, node_msg_2}, latitude, longitude, data_time_latitude_msg1,
                             data_time_longitude_msg2, false);
}

/**
 * @brief Test case for WebSocketClientCoordinatesIntegrationTest.
 *
 * This test verifies the behavior of the WebSocket client when handling messages
 * with missing coordinates. It waits for the message processing to complete and
 * checks if the messages are processed correctly.
 */
TEST_F(WebSocketClientCoordinatesIntegrationTest,
       MessageWithMissingCoordinatesWaitForMessageToComplete) {
    // Generate timestamps for each message
    std::chrono::system_clock::time_point timestamp_msg1 = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point timestamp_msg2 = timestamp_msg1 + std::chrono::seconds(1);

    // Create Metadata for each message using generated timestamps
    Metadata metadata_msg1 = Metadata(std::nullopt, timestamp_msg1);
    Metadata metadata_msg2 = Metadata(std::nullopt, timestamp_msg2);

    // Generate random coordinates
    const std::string latitude_1 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string latitude_2 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    // Create message node data for the first message, which should be ignored
    const std::vector<MessageNodeData> node_msg_1 = {
        {"CurrentLocation.Latitude", latitude_1, metadata_msg1}};

    // Create message node data for the second message
    const std::vector<MessageNodeData> node_msg_2 = {
        {"CurrentLocation.Longitude", longitude, metadata_msg2},
        {"CurrentLocation.Latitude", latitude_2, metadata_msg2}};

    // Format the timestamp for the second message
    std::string data_time_both = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", node_msg_2.at(0).metadata.getGenerated().value(), true);

    // Process and verify the messages
    processAndVerifyMessages({node_msg_1, node_msg_2}, latitude_2, longitude, data_time_both,
                             data_time_both, true);
}

/**
 * @brief Test case for verifying the processing of mixed content messages in WebSocket client.
 *
 * This test simulates the sending of mixed content messages containing latitude, longitude,
 * and speed data points. It generates random latitude and longitude values, constructs
 * message nodes with these values, and then processes and verifies the messages.
 */
TEST_F(WebSocketClientCoordinatesIntegrationTest, MixedContentMessages) {
    // Generate random latitude and longitude values as strings.
    const std::string latitude = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    // Construct message nodes with latitude, longitude, and speed data points.
    const std::vector<MessageNodeData> node_msg_1 = {
        {"CurrentLocation.Latitude", latitude, Metadata()}, {"Speed", "100", Metadata()}};
    const std::vector<MessageNodeData> node_msg_2 = {
        {"CurrentLocation.Longitude", longitude, Metadata()}, {"Speed", "200", Metadata()}};

    // Format timestamps for the latitude and longitude data points.
    std::string data_time_latitude = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", node_msg_1.at(0).metadata.getReceived(), true);
    std::string data_time_longitude = Helper::getFormattedTimestampCustom(
        "%Y-%m-%dT%H:%M:%S", node_msg_2.at(0).metadata.getReceived(), true);

    // Process and verify the constructed messages with the generated data.
    processAndVerifyMessages({node_msg_1, node_msg_2}, latitude, longitude, data_time_latitude,
                             data_time_longitude, true);
}