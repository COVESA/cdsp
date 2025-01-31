#include <iostream>
#include <string>

#include "observation_id_utils.h"
#include "random_utils.h"
#include "utc_date_utils.h"
#include "websocket_client_base_integration_test.h"

class WebSocketClientCoordinatesIntegrationTest : public WebSocketClientBaseIntegrationTest {
   protected:
    const std::string createObservationQuery(
        const std::string& vin, const std::string& observation_id, const std::string& class_name,
        const std::string& observed_property, const std::string& node_value,
        const std::string& node_type, const std::string& date_time) {
        return R"(
            PREFIX car: <http://example.ontology.com/car#>
            PREFIX sosa: <http://www.w3.org/ns/sosa/>
            PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>

            SELECT ?exists ?ntmValue WHERE {
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
               date_time + R"("^^xsd:dateTime ;
                        car:hasSimpleResultNTM ?ntmValue.
                }
                BIND(true AS ?exists)
            } LIMIT 1
        )";
    }

    // Helper function to simulate receiving messages and verify RDFox data
    void processAndVerifyMessages(
        const std::vector<std::pair<std::string, std::vector<Node>>>& messages,
        const std::string& latitude, const std::string& longitude,
        const std::string& expected_date_time, bool expect_exists,
        const int consecutive_observation_latitude = 0,
        const int consecutive_observation_longitude = 1) {
        // Add received messages
        for (const auto& [date_time, nodes] : messages) {
            mock_connection_->addReceivedMessage(
                createUpdateMessage(init_config_.oid, date_time, nodes));
        }

        // Use the inherited mockWebSocketBehavior
        mockWebSocketBehavior();

        // Verify triples in RDFox
        const std::string observation_id_latitude =
            ObservationIdentifier::createObservationIdentifier(expected_date_time,
                                                               consecutive_observation_latitude);
        const std::string observation_id_longitude =
            ObservationIdentifier::createObservationIdentifier(expected_date_time,
                                                               consecutive_observation_longitude);

        std::string sparql_query_latitude =
            createObservationQuery(init_config_.oid, observation_id_latitude, "CurrentLocation",
                                   "latitude", latitude, "double", expected_date_time);
        std::string sparql_query_longitude =
            createObservationQuery(init_config_.oid, observation_id_longitude, "CurrentLocation",
                                   "longitude", longitude, "double", expected_date_time);

        std::string result_latitude = rdfox_adapter_->queryData(sparql_query_latitude);
        std::string result_longitude = rdfox_adapter_->queryData(sparql_query_longitude);

        // Assert data existence or absence in RDFox
        if (expect_exists) {
            ASSERT_TRUE(result_latitude.find("true") != std::string::npos)
                << "Expected latitude triples not found in RDFox.";
            ASSERT_TRUE(result_longitude.find("true") != std::string::npos)
                << "Expected longitude triples not found in RDFox.";
        } else {
            ASSERT_FALSE(result_latitude.find("true") != std::string::npos)
                << "Unexpected latitude triples found in RDFox.";
            ASSERT_FALSE(result_longitude.find("true") != std::string::npos)
                << "Unexpected longitude triples found in RDFox.";
        }
    }
};

TEST_F(WebSocketClientCoordinatesIntegrationTest, CoordinatesInSameMessageGenerateTriples) {
    const std::string date_time(UtcDateUtils::generateRandomUtcDate());
    const std::string latitude = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    const std::vector<Node> nodes = {{"Vehicle.CurrentLocation.Latitude", latitude},
                                     {"Vehicle.CurrentLocation.Longitude", longitude}};

    processAndVerifyMessages({{date_time, nodes}}, latitude, longitude, date_time, true);
}

TEST_F(WebSocketClientCoordinatesIntegrationTest,
       CoordinatesInDifferentMessagesWithinTwoSecondsGenerateTriples) {
    const std::string date_time_msg1("2021-09-01T12:00:00Z");
    const std::string date_time_msg2("2021-09-01T12:00:03.000Z");  // 3 second later than message 1
    const std::string date_time_msg3("2021-09-01T12:00:04.000Z");  // 1 second later than message 2
    const std::string date_time_msg4(
        "2021-09-01T12:00:05.500Z");  // 1.5 second later than message 3

    const std::string latitude_msg1 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude_msg2 = std::to_string(RandomUtils::generateRandomDouble(-63, 86));
    const std::string longitude_msg3 = std::to_string(RandomUtils::generateRandomDouble(-63, 86));
    const std::string latitude_msg4 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));

    const std::vector<Node> node_msg_1 = {{"Vehicle.CurrentLocation.Latitude", latitude_msg1}};
    const std::vector<Node> node_msg_2 = {{"Vehicle.CurrentLocation.Longitude", longitude_msg2}};
    const std::vector<Node> node_msg_3 = {{"Vehicle.CurrentLocation.Longitude", longitude_msg3}};
    const std::vector<Node> node_msg_4 = {{"Vehicle.CurrentLocation.Latitude", latitude_msg4}};

    processAndVerifyMessages({{date_time_msg1, node_msg_1},
                              {date_time_msg2, node_msg_2},
                              {date_time_msg3, node_msg_3},
                              {date_time_msg4, node_msg_4}},
                             latitude_msg4, longitude_msg3, date_time_msg4, true);
}

TEST_F(WebSocketClientCoordinatesIntegrationTest,
       CoordinatesInDifferentMessagesAfterTwoSecondsGenerateAnyTriples) {
    const std::string date_time_msg1("2021-09-01T12:00:00Z");
    const std::string date_time_msg2("2021-09-01T12:00:03Z");  // 3 seconds later

    const std::string latitude = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    const std::vector<Node> node_msg_1 = {{"Vehicle.CurrentLocation.Latitude", latitude}};
    const std::vector<Node> node_msg_2 = {{"Vehicle.CurrentLocation.Longitude", longitude}};

    processAndVerifyMessages({{date_time_msg1, node_msg_1}, {date_time_msg2, node_msg_2}}, latitude,
                             longitude, date_time_msg2, false);
}

TEST_F(WebSocketClientCoordinatesIntegrationTest,
       MessageWithMissingCoordinatesWaitForMessageToComplete) {
    const std::string date_time_msg1("2021-09-01T12:00:00Z");
    const std::string date_time_msg2("2021-09-01T12:00:01Z");  // 1 second later

    const std::string latitude_1 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string latitude_2 = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    const std::vector<Node> node_msg_1 = {
        {"Vehicle.CurrentLocation.Latitude", latitude_1}};  // This should be ignored
    const std::vector<Node> node_msg_2 = {{"Vehicle.CurrentLocation.Longitude", longitude},
                                          {"Vehicle.CurrentLocation.Latitude", latitude_2}};

    processAndVerifyMessages({{date_time_msg1, node_msg_1}, {date_time_msg2, {node_msg_2}}},
                             latitude_2, longitude, date_time_msg2, true);
}

TEST_F(WebSocketClientCoordinatesIntegrationTest, MixedContentMessages) {
    const std::string date_time_msg1("2021-09-01T12:00:00Z");
    const std::string date_time_msg2("2021-09-01T12:00:01Z");  // 1 second later

    const std::string latitude = std::to_string(RandomUtils::generateRandomDouble(-90, 90));
    const std::string longitude = std::to_string(RandomUtils::generateRandomDouble(-63, 86));

    const std::vector<Node> node_msg_1 = {{"Vehicle.CurrentLocation.Latitude", latitude},
                                          {"Vehicle.Speed", "100"}};
    const std::vector<Node> node_msg_2 = {{"Vehicle.CurrentLocation.Longitude", longitude},
                                          {"Vehicle.Speed", "200"}};

    processAndVerifyMessages({{date_time_msg1, node_msg_1}, {date_time_msg2, node_msg_2}}, latitude,
                             longitude, date_time_msg2, true, 1, 2);
}