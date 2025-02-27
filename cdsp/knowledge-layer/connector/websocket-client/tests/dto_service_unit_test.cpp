#include <gtest/gtest.h>

#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "data_points_utils.h"
#include "dto_service.h"
#include "helper.h"
#include "metadata_dto.h"
#include "random_utils.h"
#include "test_helper.h"
#include "vin_utils.h"

class DtoServiceUnitTest
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::pair<TestHelper::DataStructure, std::string>> {
   protected:
    DtoService dto_service_;
};

INSTANTIATE_TEST_SUITE_P(
    DataStructureVariants, DtoServiceUnitTest,
    ::testing::Values(std::make_pair(TestHelper::DataStructure::Flat, "FlatNodes"),
                      std::make_pair(TestHelper::DataStructure::Nested, "NestedNodes"),
                      std::make_pair(TestHelper::DataStructure::Leaf, "LeafNodes")),
    [](const ::testing::TestParamInfo<std::pair<TestHelper::DataStructure, std::string>>& info) {
        return info.param.second;  // test name
    });

/**
 * @brief Generates a JSON object representing metadata tags for nodes.
 *
 * This function takes a map of node metadata and converts it into a JSON object.
 * Each node in the map is represented as a key in the JSON object, and its value
 * is another JSON object containing timestamp information.
 *
 * @param nodes_metadata A map where each key is a node identifier (string) and
 *                       the value is another map. This inner map's key is a
 *                       timestamp (string), and its value is a pair of integers
 *                       representing seconds and nanoseconds.
 * @return nlohmann::json A JSON object where each key is a node identifier and
 *                        its value is a JSON object containing timestamp data.
 */
nlohmann::json generateMetadataTag(
    const std::map<std::string, std::map<std::string, std::pair<int64_t, int64_t>>>&
        nodes_metadata) {
    nlohmann::json json_metadata;
    for (const auto& [node, timestamps] : nodes_metadata) {
        nlohmann::json node_metadata;
        if (!timestamps.empty()) {
            nlohmann::json timestamps_json;
            for (const auto& [timestamp, values] : timestamps) {
                timestamps_json[timestamp] = {{"seconds", values.first},
                                              {"nanoseconds", values.second}};
            }
            node_metadata["timestamps"] = timestamps_json;
        }
        json_metadata[node] = node_metadata;
    }
    return json_metadata;
}

nlohmann::json generateValidDataMessageJson() {
    return {{"type", "data"},
            {"schema", "Vehicle"},
            {"instance", "1234"},
            {"data", {{"Speed", 120}}},
            {"metadata",
             {{"Speed",
               {{"timestamps",
                 {{"generated", {{"seconds", 1234567890}, {"nanoseconds", 123456789}}}}}}}}}};
}

nlohmann::json generateValidStatusMessageJson() {
    return {{"code", 200},
            {"message", "OK"},
            {"timestamp", {{"seconds", 1234567890}, {"nanoseconds", 123456789}}}};
}

// Tests for parsing DataMessageDto

/**
 * @brief Test case for parsing a valid DataMessage JSON.
 *
 * This test verifies the parsing of a valid DataMessage JSON object into a DataMessageDto.
 * It checks the integrity and correctness of the parsing process by comparing the fields
 * of the parsed DataMessageDto with the original JSON object.
 */
TEST_P(DtoServiceUnitTest, ParseJsonDataMessage) {
    auto data_structure_type = GetParam().first;
    // Arrange
    int num_points = RandomUtils::generateRandomInt(1, 5);
    auto random_nodes = DataPointsUtils::generateDataPoints(
        data_structure_type == TestHelper::DataStructure::Leaf ? 1 : num_points);
    auto random_path = TestHelper::generatePathTag(data_structure_type);
    auto random_request_id = TestHelper::generateRequestIdTag();
    auto random_vin = VinUtils::getRandomVinString();
    auto random_json_data = TestHelper::generateDataTag(random_nodes, data_structure_type);
    std::map<std::string, std::map<std::string, std::pair<int64_t, int64_t>>> random_nodes_metadata;
    if (data_structure_type == TestHelper::DataStructure::Leaf) {
        random_nodes_metadata.insert({random_path.value(), TestHelper::generateMetadata()});
    } else {
        for (const auto& [node, _] : random_nodes) {
            random_nodes_metadata.insert({node, TestHelper::generateMetadata()});
        }
    }

    // Create dynamic incoming JSON message
    nlohmann::json json_message = {{"type", "data"},
                                   {"schema", "Vehicle"},
                                   {"instance", random_vin},
                                   {"data", random_json_data}};

    if (random_path.has_value()) {
        json_message["path"] = random_path.value();
    }
    if (!random_nodes_metadata.empty()) {
        json_message["metadata"] = generateMetadataTag(random_nodes_metadata);
    }
    if (random_request_id.has_value()) {
        json_message["requestId"] = random_request_id.value();
    }
    std::cout << "Incoming message: \n" << json_message.dump(4) << std::endl;

    // Act
    DataMessageDto dto = dto_service_.parseDataDto(json_message);
    std::cout << "Parsed DataMessageDto: \n" << dto << std::endl;

    // Assert
    ASSERT_EQ(dto.type, "data");
    ASSERT_EQ(dto.schema, "Vehicle");
    ASSERT_EQ(dto.instance, random_vin);
    ASSERT_EQ(dto.data, random_json_data);
    ASSERT_EQ(dto.path, random_path);
    ASSERT_EQ(dto.requestId, random_request_id);
    ASSERT_EQ(dto.metadata.has_value(), !random_nodes_metadata.empty());

    if (dto.metadata.has_value()) {
        ASSERT_EQ(dto.metadata.value().nodes.size(), random_nodes_metadata.size());
        for (const auto& [node, timestamps] : dto.metadata.value().nodes) {
            if (random_nodes_metadata.find(node) != random_nodes_metadata.end()) {
                if (random_nodes_metadata[node].find("generated") !=
                    random_nodes_metadata[node].end()) {
                    ASSERT_EQ(timestamps.generated.seconds,
                              random_nodes_metadata[node]["generated"].first);
                    ASSERT_EQ(timestamps.generated.nanoseconds,
                              random_nodes_metadata[node]["generated"].second);
                }
                if (random_nodes_metadata[node].find("received") !=
                    random_nodes_metadata[node].end()) {
                    ASSERT_EQ(timestamps.received.seconds,
                              random_nodes_metadata[node]["received"].first);
                    ASSERT_EQ(timestamps.received.nanoseconds,
                              random_nodes_metadata[node]["received"].second);
                }
            }
        }
    }
}

/**
 * @brief Unit test for the DtoService class to verify exception handling.
 *
 * This test case checks that the `parseDataDto` method of the `dto_service_` object
 * throws an `std::invalid_argument` exception when any of the required fields are missing
 * from the JSON message. The required fields are "type", "schema", "instance", and "data".
 *
 * The test iterates over each required field, removes it from a valid JSON message,
 * and asserts that an exception is thrown when the modified message is parsed.
 */
TEST_F(DtoServiceUnitTest, ParseDataDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
    // Define the complete JSON message
    nlohmann::json json_message = generateValidDataMessageJson();

    // List of required fields in the JSON message
    std::vector<std::string> required_fields = {"type", "schema", "instance", "data"};

    // Iterate over each required field
    for (const auto& field : required_fields) {
        nlohmann::json test_message = json_message;
        test_message.erase(field);  // Remove the required field

        // Output the current test case details
        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << test_message.dump(4) << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_service_.parseDataDto(test_message), std::invalid_argument);
    }
}

/**
 * @brief Unit test for the DtoService class to verify exception handling for incomplete metadata.
 *
 * This test case checks that the `parseDataDto` method of the `dto_service_` object
 * throws an `std::invalid_argument` exception when any of the required fields in the metadata
 * are missing from the JSON message. The required fields within the "generated" timestamps
 * are "seconds" and "nanoseconds".
 *
 * The test iterates over each required field, removes it from a valid JSON message's metadata,
 * and asserts that an exception is thrown when the modified message is parsed.
 */
TEST_F(DtoServiceUnitTest, ParseDataDtoThrowsExceptionWhenMetadataIsIncomplete) {
    // Define the complete JSON message
    nlohmann::json json_message = generateValidDataMessageJson();

    // List of required fields in the metadata's "generated" timestamps
    std::vector<std::string> required_fields = {"seconds", "nanoseconds"};

    // Iterate over each required field
    for (const auto& field : required_fields) {
        nlohmann::json test_message = json_message;
        test_message["metadata"]["Speed"]["timestamps"]["generated"].erase(
            field);  // Remove the required field

        // Output the current test case details
        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << test_message.dump(4) << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_service_.parseDataDto(test_message), std::invalid_argument);
    }
}

/**
 * @brief Unit test for DtoService to verify that an exception is thrown when the path is missing in
 * the JSON message and data is not an object.
 *
 * This test constructs a JSON message with missing path information and verifies that the
 * parseDataDto function throws an std::invalid_argument exception when data is not an object. The
 * JSON message includes a type, schema, instance, and data, but lacks the necessary path field,
 * which should trigger the exception.
 */
TEST_F(DtoServiceUnitTest, ParseDataDtoThrowsExceptionWhenPathIsMissing) {
    // Define the complete JSON message
    nlohmann::json json_message = {
        {"type", "data"}, {"schema", "Vehicle"}, {"instance", "1234"}, {"data", 120}
        // Data is not an object
    };

    std::cout << "\nTesting with missing path\n" << json_message.dump(4) << std::endl;

    // Act & Assert: Ensure exception is thrown when path is missing
    ASSERT_THROW(dto_service_.parseDataDto(json_message), std::invalid_argument);
}

// Test for parsing a Status Message JSON

/**
 * @brief Test case for parsing a valid StatusMessage JSON.
 *
 * This test verifies the parsing of a valid StatusMessage JSON object into a StatusMessageDto.
 * It checks the integrity and correctness of the parsing process by comparing the fields
 * of the parsed StatusMessageDto with the original JSON object.
 */
TEST_F(DtoServiceUnitTest, ParseStatusMessage) {
    // Arrange
    int random_code = RandomUtils::generateRandomInt(100, 999);
    std::string random_message = RandomUtils::generateRandomString(10);
    auto random_timestamps = TestHelper::getSecondsAndNanoseconds();
    auto random_requestId = TestHelper::generateRequestIdTag();

    nlohmann::json json_message = {
        {"code", random_code},
        {"message", random_message},
        {"timestamp",
         {{"seconds", random_timestamps.first}, {"nanoseconds", random_timestamps.second}}}};
    if (random_requestId.has_value()) {
        json_message["requestId"] = random_requestId.value();
    }

    std::cout << "Incoming message: \n" << json_message.dump(4) << std::endl;

    // Act
    StatusMessageDto dto = dto_service_.parseStatusDto(json_message);
    std::cout << "Parsed StatusMessageDto: \n" << dto << std::endl;

    // Assert
    ASSERT_EQ(dto.code, random_code);
    ASSERT_EQ(dto.message, random_message);
    ASSERT_EQ(dto.requestId, random_requestId);
    ASSERT_EQ(dto.timestamp.seconds, random_timestamps.first);
    ASSERT_EQ(dto.timestamp.nanoseconds, random_timestamps.second);
}

/**
 * @brief Unit test for the DtoService class to verify exception handling for missing required
 * fields for the status message.
 *
 * This test case checks that the `parseStatusDto` method of the `dto_service_` object
 * throws an `std::invalid_argument` exception when any of the required fields are missing
 * from the JSON message. The required fields are "code", "message", and "timestamp".
 *
 * The test iterates over each required field, removes it from a valid JSON message,
 * and asserts that an exception is thrown when the modified message is parsed.
 */
TEST_F(DtoServiceUnitTest, ParseStatusDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
    // Define the complete JSON message
    nlohmann::json json_message = generateValidStatusMessageJson();

    // List of required fields in the JSON message
    std::vector<std::string> required_fields = {"code", "message", "timestamp"};

    // Iterate over each required field
    for (const auto& field : required_fields) {
        nlohmann::json test_message = json_message;
        test_message.erase(field);  // Remove the required field

        // Output the current test case details
        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << test_message.dump(4) << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_service_.parseStatusDto(test_message), std::invalid_argument);
    }
}

/**
 * @brief Unit test for the DtoService class to verify exception handling for incomplete timestamps.
 *
 * This test case checks that the `parseStatusDto` method of the `dto_service_` object
 * throws an `std::invalid_argument` exception when any of the required fields in the timestamp
 * are missing from the JSON message. The required fields within the "timestamp" are "seconds" and
 * "nanoseconds".
 *
 * The test iterates over each required field, removes it from a valid JSON message's timestamp,
 * and asserts that an exception is thrown when the modified message is parsed.
 */
TEST_F(DtoServiceUnitTest, ParseStatusDtoThrowsExceptionWhenTimestampIsIncomplete) {
    // Define the complete JSON message
    nlohmann::json json_message = generateValidStatusMessageJson();

    // List of required fields in the timestamp
    std::vector<std::string> required_fields = {"seconds", "nanoseconds"};

    // Iterate over each required field
    for (const auto& field : required_fields) {
        nlohmann::json test_message = json_message;
        test_message["timestamp"].erase(field);  // Remove the required field

        // Output the current test case details
        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << test_message.dump(4) << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_service_.parseStatusDto(test_message), std::invalid_argument);
    }
}
