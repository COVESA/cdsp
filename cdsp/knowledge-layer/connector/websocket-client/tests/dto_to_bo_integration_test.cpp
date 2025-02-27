#include <gtest/gtest.h>

#include <utility>

#include "data_points_utils.h"
#include "dto_to_bo.h"
#include "helper.h"
#include "random_utils.h"
#include "test_helper.h"
#include "vin_utils.h"
class DtoToBoIntegrationTest
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::pair<TestHelper::DataStructure, std::string>> {
   protected:
    DtoToBo dto_to_bo_;
    std::chrono::system_clock::time_point test_execution_start_point_;
    const SchemaType SCHEMA_TYPE = SchemaType::VEHICLE;

    void SetUp() override { test_execution_start_point_ = std::chrono::system_clock::now(); }
};

INSTANTIATE_TEST_SUITE_P(
    DataStructureVariants, DtoToBoIntegrationTest,
    ::testing::Values(std::make_pair(TestHelper::DataStructure::Flat, "FlatNodes"),
                      std::make_pair(TestHelper::DataStructure::Nested, "NestedNodes"),
                      std::make_pair(TestHelper::DataStructure::Leaf, "LeafNodes")),
    [](const ::testing::TestParamInfo<std::pair<TestHelper::DataStructure, std::string>>& info) {
        return info.param.second;  // test name
    });

/**
 * @brief Generates a MetadataDto object from a map of node metadata.
 *
 * This function processes a map containing metadata for various nodes, where each node
 * is associated with a map of timestamp pairs. It extracts the "generated" and "received"
 * timestamps for each node and constructs a MetadataDto object. If no nodes are present
 * in the input map, the function returns an empty optional.
 *
 * @param nodes_metadata A map where the key is a node identifier (string) and the value
 * is another map. This inner map has keys as tags ("generated" or "received") and values
 * as pairs of integers representing seconds and nanoseconds.
 *
 * @return An optional MetadataDto object containing the processed metadata. Returns
 * std::nullopt if no nodes are present in the input map.
 */
std::optional<MetadataDto> generateMetadataDto(
    const std::map<std::string, std::map<std::string, std::pair<int64_t, int64_t>>>&
        nodes_metadata) {
    MetadataDto metadata_dto;
    for (const auto& [node, timestamps] : nodes_metadata) {
        MetadataDto::NodeMetadata node_metadata;
        for (const auto& [tag, timestamp] : timestamps) {
            if (tag == "generated") {
                node_metadata.generated.seconds = timestamp.first;
                node_metadata.generated.nanoseconds = timestamp.second;
            } else if (tag == "received") {
                node_metadata.received.seconds = timestamp.first;
                node_metadata.received.nanoseconds = timestamp.second;
            }
        }
        metadata_dto.nodes[node] = node_metadata;
    }
    if (metadata_dto.nodes.empty()) {
        return std::nullopt;
    }
    return metadata_dto;
}

std::map<SchemaType, std::vector<std::string>> generateSystemDataPoints(
    const SchemaType& schema,
    const std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>&
        nodes) {
    std::map<SchemaType, std::vector<std::string>> system_data_points;
    for (const auto& [node, _] : nodes) {
        const auto supported_node = SchemaTypeToString(schema) + "." + node;
        system_data_points[schema].push_back(supported_node);
    }

    return system_data_points;
}

/**
 * @brief Converts a std::variant containing different types to a std::string.
 *
 * This function takes a std::variant that can hold a std::string, int, double, float, or bool,
 * and converts the contained value to a std::string. It uses std::visit to apply a lambda
 * function that handles each possible type:
 * - If the value is a std::string, it is returned directly.
 * - If the value is a bool, it is converted to "true" or "false".
 * - If the value is a numeric type (int, double, float), it is converted using std::to_string.
 *
 * @param var A std::variant containing a value of type std::string, int, double, float, or bool.
 *
 * @return A std::string representation of the value contained in the variant.
 */
std::string variantToString(const std::variant<std::string, int, double, float, bool>& var) {
    return std::visit(
        [](const auto& value) -> std::string {
            if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>) {
                return value;  // Return string directly
            } else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, bool>) {
                return value ? "true" : "false";  // Convert boolean to string
            } else {
                return std::to_string(value);  // Convert numbers using std::to_string
            }
        },
        var);
}

/**
 * @brief Assembles and returns a pair consisting of a DataMessageDto and a map of system data
 * points.
 *
 * This function creates a DataMessageDto object with predefined values for type, schema, instance,
 * and data. It also initializes a MetadataDto object for the metadata field. Additionally, it
 * constructs a map of system data points, associating a vector of strings with a SchemaType key.
 * The function returns these two objects as a pair.
 *
 * @return A std::pair containing:
 * - A DataMessageDto object with initialized fields.
 * - A std::map where the key is of type SchemaType and the value is a vector of strings
 * representing system data points.
 */
std::pair<DataMessageDto, std::map<SchemaType, std::vector<std::string>>>
assembleValidFunctionArgs() {
    DataMessageDto dto;
    dto.type = "data";
    dto.schema = "Vehicle";
    dto.instance = "sampleVin";
    dto.data = {{"node_name", "value"}};
    dto.metadata = MetadataDto();

    // Initialize system data points map
    std::map<SchemaType, std::vector<std::string>> system_data_points;
    system_data_points[SchemaType::VEHICLE].push_back("Vehicle.node_name");

    // Return the assembled DataMessageDto and supported data points
    return {dto, system_data_points};
}

// Test for converting DataMessageDto to DataMessage

/**
 * @brief Test case for converting a DataMessageDto to a DataMessage business object.
 *
 * This test verifies the conversion of a DataMessageDto to a DataMessage business object (BO).
 * It checks the integrity and correctness of the conversion process by comparing the fields
 * and metadata of the resulting BO with the expected values derived from the DTO.
 */
TEST_P(DtoToBoIntegrationTest, ConvertDataMessageDtoToBo) {
    // Arrange
    auto data_structure_type = GetParam().first;

    int num_points = RandomUtils::generateRandomInt(1, 5);
    auto random_nodes = DataPointsUtils::generateDataPoints(
        data_structure_type == TestHelper::DataStructure::Leaf ? 1 : num_points);
    auto random_json_data = TestHelper::generateDataTag(random_nodes, data_structure_type);
    auto random_path = TestHelper::generatePathTag(data_structure_type);
    auto random_vin = VinUtils::getRandomVinString();

    std::map<std::string, std::map<std::string, std::pair<int64_t, int64_t>>> random_nodes_metadata;
    std::map<SchemaType, std::vector<std::string>> random_system_data_points;

    if (data_structure_type == TestHelper::DataStructure::Leaf) {
        random_nodes_metadata.insert({random_path.value(), TestHelper::generateMetadata()});
        const auto supported_node =
            SchemaTypeToString(SCHEMA_TYPE, true) + "." + random_path.value();

        random_system_data_points[SCHEMA_TYPE].push_back(supported_node);
    } else {
        for (const auto& [node, _] : random_nodes) {
            random_nodes_metadata.insert({node, TestHelper::generateMetadata()});
            const auto supported_node = SchemaTypeToString(SCHEMA_TYPE, true) + "." +
                                        (random_path.has_value() ? random_path.value() + "." : "") +
                                        node;

            random_system_data_points[SCHEMA_TYPE].push_back(supported_node);
        }
    }
    auto random_request_id = TestHelper::generateRequestIdTag();

    // Create dynamic DataMessageDto
    DataMessageDto dto;
    dto.type = "data";
    dto.schema = SchemaTypeToString(SCHEMA_TYPE, true);
    dto.instance = random_vin;
    dto.data = random_json_data;
    dto.path = random_path;
    dto.metadata = generateMetadataDto(random_nodes_metadata);
    dto.requestId = random_request_id;

    std::cout << "DataMessageDto to parse:\n" << dto << std::endl;

    // Act
    DataMessage bo = dto_to_bo_.convert(dto, random_system_data_points);
    std::cout << "DataMessage parsed:\n" << bo << std::endl;

    // Assert
    ASSERT_EQ(bo.getHeader().getId(), dto.instance);
    ASSERT_EQ(bo.getHeader().getSchemaType(), SchemaType::VEHICLE);
    ASSERT_EQ(bo.getNodes().size(), random_nodes.size());
    for (const auto& bo_node : bo.getNodes()) {
        const auto bo_node_name = bo_node.getName();
        const auto bo_node_value = bo_node.getValue().value();
        std::string random_metadata_node_name;
        if (data_structure_type == TestHelper::DataStructure::Leaf) {
            random_metadata_node_name = dto.path.value();
            ASSERT_EQ(bo_node_name, SchemaTypeToString(SCHEMA_TYPE, true) + "." + dto.path.value());
            ASSERT_EQ(bo_node_value, Helper::jsonToString(dto.data));
        } else {
            random_metadata_node_name = dto.path.has_value() ? dto.path.value() + "." : "";
            std::string expected_random_node_name = bo_node_name.substr(
                (SchemaTypeToString(SCHEMA_TYPE, true) + "." + random_metadata_node_name).size());

            ASSERT_TRUE(random_nodes.find(expected_random_node_name) != random_nodes.end());
            ASSERT_EQ(variantToString(random_nodes[expected_random_node_name]), bo_node_value);
        }

        std::string expected_random_metadata_node_name =
            bo_node_name.substr((SchemaTypeToString(SCHEMA_TYPE, true) + ".").size());
        if (dto.metadata.has_value()) {
            if (dto.metadata.value().nodes.find(expected_random_metadata_node_name) !=
                dto.metadata.value().nodes.end()) {
                auto dto_time = dto.metadata.value().nodes.at(expected_random_metadata_node_name);
                if (dto_time.generated.seconds != 0 || dto_time.generated.nanoseconds != 0) {
                    auto timestamp_from_dto = Helper::convertToTimestamp(
                        dto_time.generated.seconds, dto_time.generated.nanoseconds);

                    ASSERT_EQ(timestamp_from_dto, bo_node.getMetadata().getGenerated());
                } else {
                    ASSERT_FALSE(bo_node.getMetadata().getGenerated().has_value());
                }
                if (dto_time.received.seconds != 0 || dto_time.received.nanoseconds != 0) {
                    auto timestamp_from_dto = Helper::convertToTimestamp(
                        dto_time.received.seconds, dto_time.received.nanoseconds);

                    ASSERT_EQ(bo_node.getMetadata().getReceived(), timestamp_from_dto);
                } else {
                    // If received time is not set, it will be assigned the current time when
                    // the message was processed and should be after the test execution start
                    // point
                    ASSERT_TRUE(bo_node.getMetadata().getReceived() > test_execution_start_point_);
                }
            }
        } else {
            // If any time has been set, the current time when the message was processed will be
            // assigned to received and it should be after the test execution start point
            ASSERT_TRUE(bo_node.getMetadata().getReceived() > test_execution_start_point_);
        }
    }
}

/**
 * @brief Test case for converting a DataMessageDto with a missing path and non-object data.
 *
 * This test verifies that attempting to convert a DataMessageDto to a business object (BO)
 * throws an invalid_argument exception when the path is missing and the data is not an object.
 * It ensures that the conversion process correctly handles cases where essential fields are
 * improperly formatted or missing.
 */
TEST_F(DtoToBoIntegrationTest,
       ConvertDataMessageDtoThrowsExceptionWhenPathIsMissingAndDataIsNotObject) {
    // Define the system data points
    std::map<SchemaType, std::vector<std::string>> system_data_points = {
        {SCHEMA_TYPE, {"Vehicle.some_node"}}};

    // Create a DataMessageDto with missing path and non-object data
    DataMessageDto dto;
    dto.type = "data";            // Set the type of the message
    dto.schema = "Vehicle";       // Define the schema as "Vehicle"
    dto.instance = "sampleVin";   // Set a sample VIN as the instance
    dto.data = "some_value";      // Data must be an object if path is missing
    dto.metadata = std::nullopt;  // Metadata is not provided

    // Randomly decide if the path should be explicitly empty
    if (RandomUtils::generateRandomBool()) {
        dto.path = std::string();  // Empty path is also considered as missing
    }

    // Output the DTO to be parsed
    std::cout << "DataMessageDto to parse with missing path:\n" << dto << std::endl;

    // Assert that converting the DTO throws an invalid_argument exception due to the missing path
    ASSERT_THROW(dto_to_bo_.convert(dto, system_data_points), std::invalid_argument);
}

/**
 * @brief Test case for converting a DataMessageDto with missing required fields.
 *
 * This test verifies that attempting to convert a DataMessageDto to a business object (BO)
 * throws an invalid_argument exception when any of the required fields are missing.
 * It ensures that the conversion process correctly handles cases where essential fields are
 * improperly formatted or missing.
 */
TEST_F(DtoToBoIntegrationTest, ConvertDataMessageDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
    // Define the complete DataMessageDto
    std::map<SchemaType, std::vector<std::string>> system_data_points = {
        {SCHEMA_TYPE, {"Vehicle.some_node"}}};

    std::vector<std::string> required_fields = {"type", "schema", "instance", "data",
                                                "supported_system_data_points"};

    for (const auto& field : required_fields) {
        auto [dto, system_data_points] = assembleValidFunctionArgs();
        if (field == "type") {
            dto.type = std::string();
        } else if (field == "schema") {
            dto.schema = std::string();
        } else if (field == "instance") {
            dto.instance = std::string();
        } else if (field == "data") {
            dto.data = nlohmann::json();
        } else if (field == "supported_system_data_points") {
            system_data_points.clear();
        }

        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << dto << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_to_bo_.convert(dto, system_data_points), std::invalid_argument);
    }
}

/**
 * @brief Test case for converting a DataMessageDto with incomplete metadata.
 *
 * This test verifies that attempting to convert a DataMessageDto to a business object (BO)
 * does not throw an exception when the metadata is incomplete.
 * It ensures that the conversion process correctly handles cases where metadata fields are
 * improperly formatted or missing.
 */
TEST_F(DtoToBoIntegrationTest, ConvertDataMessageDtoNotThrowExceptionWhenMetadataIsWrong) {
    // Define the complete DataMessageDto
    auto [dto, system_data_points] = assembleValidFunctionArgs();

    std::vector<std::string> required_fields = {"seconds", "nanoseconds"};
    std::string time_type = RandomUtils::generateRandomBool() ? "generated" : "received";

    for (const auto& field : required_fields) {
        dto.metadata->nodes["node_name"].generated.seconds = 0;
        dto.metadata->nodes["node_name"].generated.nanoseconds = 0;
        dto.metadata->nodes["node_name"].received.seconds = 0;
        dto.metadata->nodes["node_name"].received.nanoseconds = 0;
        dto.metadata->nodes["node_name"].generated.nanoseconds = 0;

        // Set wrong field value
        if (field == "seconds") {
            time_type == "generated" ? dto.metadata->nodes["node_name"].generated.seconds = -1
                                     : dto.metadata->nodes["node_name"].received.seconds = -1;
        } else if (field == "nanoseconds") {
            time_type == "generated" ? dto.metadata->nodes["node_name"].generated.nanoseconds = -1
                                     : dto.metadata->nodes["node_name"].received.nanoseconds = -1;
        }

        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << dto << std::endl;

        // Act
        DataMessage bo = dto_to_bo_.convert(dto, system_data_points);
        std::cout << "DataMessage parsed:\n" << bo << std::endl;

        if (time_type == "generated") {
            // Assert: Ensure the generated time is not set
            ASSERT_FALSE(bo.getNodes().front().getMetadata().getGenerated().has_value());
        } else {
            // Assert: Ensure the received time is set to the current time
            ASSERT_TRUE(bo.getNodes().front().getMetadata().getReceived() >
                        test_execution_start_point_);
        }
    }
}

/**
 * @brief Test case for converting a DataMessageDto with an unsupported data point.
 *
 * This test verifies that attempting to convert a DataMessageDto to a business object (BO)
 * does log the unsupported node but does not throw an exception when a data point is not supported.
 */
TEST_F(DtoToBoIntegrationTest,
       ConvertDataMessageDtoNotThrowsExceptionWhenADataPointIsNotSupported) {
    // Define the complete DataMessageDto
    auto [dto, system_data_points] = assembleValidFunctionArgs();

    // Add an unsupported data point to the DTO, which is not included in the system data points
    dto.data["unsupported_node"] = "value";

    // Output the DTO to be parsed
    std::cout << "DataMessageDto to parse with missing supported data points:\n"
              << dto << std::endl;

    DataMessage bo = dto_to_bo_.convert(dto, system_data_points);
    std::cout << "DataMessage parsed:\n" << bo << std::endl;

    // Assert: Ensure the unsupported data point is not included in the BO
    ASSERT_EQ(bo.getNodes().size(), 1);  // Only the supported data point should be included
    ASSERT_EQ(bo.getNodes().front().getName(), "Vehicle.node_name");
    ASSERT_EQ(bo.getNodes().front().getValue().value(), "value");
}

/**
 * @brief Test case for converting a StatusMessageDto to a StatusMessage business object.
 *
 * This test verifies that the conversion from a StatusMessageDto to a StatusMessage
 * business object is performed correctly. It checks that all fields are accurately
 * transferred from the DTO to the BO.
 */
TEST_F(DtoToBoIntegrationTest, ConvertStatusMessageDtoToBo) {
    // Arrange
    int random_code = RandomUtils::generateRandomInt(100, 999);
    std::string random_message = RandomUtils::generateRandomString(10);
    auto random_timestamps = TestHelper::getSecondsAndNanoseconds();
    auto random_requestId = TestHelper::generateRequestIdTag();

    // Create dynamic StatusMessageDto
    StatusMessageDto dto;
    dto.code = random_code;
    dto.message = random_message;
    dto.timestamp.seconds = random_timestamps.first;
    dto.timestamp.nanoseconds = random_timestamps.second;
    dto.requestId = random_requestId;

    std::cout << "StatusMessageDto to parse:\n" << dto << std::endl;

    // Act
    StatusMessage bo = dto_to_bo_.convert(dto);
    std::cout << "StatusMessage parsed:\n" << bo << std::endl;

    // Assert
    ASSERT_EQ(bo.getCode(), dto.code);
    ASSERT_EQ(bo.getMessage(), dto.message);
    ASSERT_EQ(bo.getRequestId(), dto.requestId);
    ASSERT_EQ(bo.getTimestamp(),
              Helper::convertToTimestamp(dto.timestamp.seconds, dto.timestamp.nanoseconds));
}

/**
 * @brief Test case for converting a StatusMessageDto with missing required fields.
 *
 * This test verifies that attempting to convert a StatusMessageDto to a business object (BO)
 * throws an invalid_argument exception when any of the required fields are missing.
 */
TEST_F(DtoToBoIntegrationTest, ConvertStatusMessageDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
    std::vector<std::string> required_fields = {"message", "timestamp"};

    for (const auto& field : required_fields) {
        // Define the complete StatusMessageDto
        StatusMessageDto dto;
        dto.code = 200;
        dto.message = "OK";
        dto.timestamp.seconds = 1234567890;
        dto.timestamp.nanoseconds = 123456789;

        if (field == "message") {
            dto.message = std::string();
        } else if (field == "timestamp") {
            dto.timestamp.seconds = 0;
            dto.timestamp.nanoseconds = 0;
        }

        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << dto << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_to_bo_.convert(dto), std::invalid_argument);
    }
}