#include <gtest/gtest.h>

#include <utility>

#include "data_points_utils.h"
#include "dto_to_bo.h"
#include "helper.h"
#include "random_utils.h"
#include "test_helper.h"
#include "vin_utils.h"
class DtoToDataMessageIntegrationTest
    : public ::testing::Test,
      public ::testing::WithParamInterface<std::pair<TestHelper::DataStructure, std::string>> {
   protected:
    DtoToBo dto_to_bo_;
    std::chrono::system_clock::time_point test_execution_start_point_;
    const SchemaType SCHEMA_TYPE = SchemaType::VEHICLE;

    void SetUp() override { test_execution_start_point_ = std::chrono::system_clock::now(); }
};

INSTANTIATE_TEST_SUITE_P(
    DataStructureVariants, DtoToDataMessageIntegrationTest,
    ::testing::Values(std::make_pair(TestHelper::DataStructure::Flat, "FlatNodes"),
                      std::make_pair(TestHelper::DataStructure::Nested, "NestedNodes"),
                      std::make_pair(TestHelper::DataStructure::Leaf, "LeafNodes")),
    [](const ::testing::TestParamInfo<std::pair<TestHelper::DataStructure, std::string>>& info) {
        return info.param.second;  // test name
    });

/**
 * @brief Generates a MetadataDTO object from a map of node metadata.
 *
 * This function processes a map containing metadata for various nodes, where each node
 * is associated with a map of timestamp pairs. It extracts the "generated" and "received"
 * timestamps for each node and constructs a MetadataDTO object. If no nodes are present
 * in the input map, the function returns an empty optional.
 *
 * @param nodes_metadata A map where the key is a node identifier (string) and the value
 * is another map. This inner map has keys as tags ("generated" or "received") and values
 * as pairs of integers representing seconds and nanos.
 *
 * @return An optional MetadataDTO object containing the processed metadata. Returns
 * std::nullopt if no nodes are present in the input map.
 */
std::optional<MetadataDTO> generateMetadataDTO(
    const std::map<std::string, std::map<std::string, std::pair<int64_t, int64_t>>>&
        nodes_metadata) {
    MetadataDTO metadata_dto;
    for (const auto& [node, timestamps] : nodes_metadata) {
        MetadataDTO::NodeMetadata node_metadata;
        for (const auto& [tag, timestamp] : timestamps) {
            if (tag == "generated") {
                node_metadata.generated.seconds = timestamp.first;
                node_metadata.generated.nanos = timestamp.second;
            } else if (tag == "received") {
                node_metadata.received.seconds = timestamp.first;
                node_metadata.received.nanos = timestamp.second;
            }
        }
        metadata_dto.nodes[node] = node_metadata;
    }
    if (metadata_dto.nodes.empty()) {
        return std::nullopt;
    }
    return metadata_dto;
}

/**
 * @brief Assembles and returns a pair consisting of a DataMessageDTO and a map of system data
 * points.
 *
 * This function creates a DataMessageDTO object with predefined values for type, schema, instance,
 * and data. It also initializes a MetadataDTO object for the metadata field. Additionally, it
 * constructs a map of system data points, associating a vector of strings with a SchemaType key.
 * The function returns these two objects as a pair.
 *
 * @return A std::pair containing:
 * - A DataMessageDTO object with initialized fields.
 * - A std::map where the key is of type SchemaType and the value is a vector of strings
 * representing system data points.
 */
std::pair<DataMessageDTO, std::map<SchemaType, std::vector<std::string>>>
assembleValidFunctionArgs() {
    DataMessageDTO dto;
    dto.type = "data";
    dto.schema = "Vehicle";
    dto.instance = "sampleVin";
    dto.data = {{"node_name", "value"}};
    dto.metadata = MetadataDTO();

    // Initialize system data points map
    std::map<SchemaType, std::vector<std::string>> system_data_points;
    system_data_points[SchemaType::VEHICLE].push_back("Vehicle.node_name");

    // Return the assembled DataMessageDTO and supported data points
    return {dto, system_data_points};
}

// Test for converting DataMessageDTO to DataMessage

/**
 * @brief Test case for converting a DataMessageDTO to a DataMessage business object.
 *
 * This test verifies the conversion of a DataMessageDTO to a DataMessage business object (BO).
 * It checks the integrity and correctness of the conversion process by comparing the fields
 * and metadata of the resulting BO with the expected values derived from the DTO.
 */
TEST_P(DtoToDataMessageIntegrationTest, ConvertDataMessageDTOToBo) {
    // Arrange
    auto data_structure_type = GetParam().first;

    int num_points = RandomUtils::generateRandomInt(1, 5);
    auto random_nodes = DataPointsUtils::generateDataPointsWithValues(
        data_structure_type == TestHelper::DataStructure::Leaf ? 1 : num_points);
    auto random_json_data = TestHelper::generateDataTag(random_nodes, data_structure_type);
    auto random_path = TestHelper::generatePathTag(data_structure_type);
    auto random_vin = VinUtils::getRandomVinString();

    std::map<std::string, std::map<std::string, std::pair<int64_t, int64_t>>> random_nodes_metadata;
    std::map<SchemaType, std::vector<std::string>> random_system_data_points;

    if (data_structure_type == TestHelper::DataStructure::Leaf) {
        random_nodes_metadata.insert({random_path.value(), TestHelper::generateMetadata()});
        const auto supported_node =
            schemaTypeToString(SCHEMA_TYPE, true) + "." + random_path.value();

        random_system_data_points[SCHEMA_TYPE].push_back(supported_node);
    } else {
        for (const auto& [node, _] : random_nodes) {
            random_nodes_metadata.insert({node, TestHelper::generateMetadata()});
            const auto supported_node = schemaTypeToString(SCHEMA_TYPE, true) + "." +
                                        (random_path.has_value() ? random_path.value() + "." : "") +
                                        node;

            random_system_data_points[SCHEMA_TYPE].push_back(supported_node);
        }
    }
    auto random_request_id = TestHelper::generateRequestIdTag();

    // Create dynamic DataMessageDTO
    DataMessageDTO dto;
    dto.type = "data";
    dto.schema = schemaTypeToString(SCHEMA_TYPE, true);
    dto.instance = random_vin;
    dto.data = random_json_data;
    dto.path = random_path;
    dto.metadata = generateMetadataDTO(random_nodes_metadata);
    dto.requestId = random_request_id;

    std::cout << "DataMessageDTO to parse:\n" << dto << std::endl;

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
            ASSERT_EQ(bo_node_name, schemaTypeToString(SCHEMA_TYPE, true) + "." + dto.path.value());
            ASSERT_EQ(bo_node_value, Helper::jsonToString(dto.data));
        } else {
            random_metadata_node_name = dto.path.has_value() ? dto.path.value() + "." : "";
            std::string expected_random_node_name = bo_node_name.substr(
                (schemaTypeToString(SCHEMA_TYPE, true) + "." + random_metadata_node_name).size());

            ASSERT_TRUE(random_nodes.find(expected_random_node_name) != random_nodes.end());
            ASSERT_EQ(Helper::variantToString(random_nodes[expected_random_node_name]),
                      bo_node_value);
        }

        std::string expected_random_metadata_node_name =
            bo_node_name.substr((schemaTypeToString(SCHEMA_TYPE, true) + ".").size());
        if (dto.metadata.has_value()) {
            if (dto.metadata.value().nodes.find(expected_random_metadata_node_name) !=
                dto.metadata.value().nodes.end()) {
                auto dto_time = dto.metadata.value().nodes.at(expected_random_metadata_node_name);
                if (dto_time.generated.seconds != 0 || dto_time.generated.nanos != 0) {
                    auto timestamp_from_dto = Helper::convertToTimestamp(dto_time.generated.seconds,
                                                                         dto_time.generated.nanos);

                    ASSERT_EQ(timestamp_from_dto, bo_node.getMetadata().getGenerated());
                } else {
                    ASSERT_FALSE(bo_node.getMetadata().getGenerated().has_value());
                }
                if (dto_time.received.seconds != 0 || dto_time.received.nanos != 0) {
                    auto timestamp_from_dto = Helper::convertToTimestamp(dto_time.received.seconds,
                                                                         dto_time.received.nanos);

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
 * @brief Test case for converting a DataMessageDTO with a missing path and non-object data.
 *
 * This test verifies that attempting to convert a DataMessageDTO to a business object (BO)
 * throws an invalid_argument exception when the path is missing and the data is not an object.
 * It ensures that the conversion process correctly handles cases where essential fields are
 * improperly formatted or missing.
 */
TEST_F(DtoToDataMessageIntegrationTest,
       ConvertDataMessageDTOThrowsExceptionWhenPathIsMissingAndDataIsNotObject) {
    // Define the system data points
    std::map<SchemaType, std::vector<std::string>> system_data_points = {
        {SCHEMA_TYPE, {"Vehicle.some_node"}}};

    // Create a DataMessageDTO with missing path and non-object data
    DataMessageDTO dto;
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
    std::cout << "DataMessageDTO to parse with missing path:\n" << dto << std::endl;

    // Assert that converting the DTO throws an invalid_argument exception due to the missing path
    ASSERT_THROW(dto_to_bo_.convert(dto, system_data_points), std::invalid_argument);
}

/**
 * @brief Test case for converting a DataMessageDTO with missing required fields.
 *
 * This test verifies that attempting to convert a DataMessageDTO to a business object (BO)
 * throws an invalid_argument exception when any of the required fields are missing.
 * It ensures that the conversion process correctly handles cases where essential fields are
 * improperly formatted or missing.
 */
TEST_F(DtoToDataMessageIntegrationTest,
       ConvertDataMessageDTOThrowsExceptionWhenRequiredFieldsAreMissing) {
    // Define the complete DataMessageDTO
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
 * @brief Test case for converting a DataMessageDTO with incomplete metadata.
 *
 * This test verifies that attempting to convert a DataMessageDTO to a business object (BO)
 * does not throw an exception when the metadata is incomplete.
 * It ensures that the conversion process correctly handles cases where metadata fields are
 * improperly formatted or missing.
 */
TEST_F(DtoToDataMessageIntegrationTest, ConvertDataMessageDTONotThrowExceptionWhenMetadataIsWrong) {
    // Define the complete DataMessageDTO
    auto [dto, system_data_points] = assembleValidFunctionArgs();

    std::vector<std::string> required_fields = {"seconds", "nanos"};
    std::string time_type = RandomUtils::generateRandomBool() ? "generated" : "received";

    for (const auto& field : required_fields) {
        dto.metadata->nodes["node_name"].generated.seconds = 0;
        dto.metadata->nodes["node_name"].generated.nanos = 0;
        dto.metadata->nodes["node_name"].received.seconds = 0;
        dto.metadata->nodes["node_name"].received.nanos = 0;
        dto.metadata->nodes["node_name"].generated.nanos = 0;

        // Set wrong field value
        if (field == "seconds") {
            time_type == "generated" ? dto.metadata->nodes["node_name"].generated.seconds = -1
                                     : dto.metadata->nodes["node_name"].received.seconds = -1;
        } else if (field == "nanos") {
            time_type == "generated" ? dto.metadata->nodes["node_name"].generated.nanos = -1
                                     : dto.metadata->nodes["node_name"].received.nanos = -1;
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
 * @brief Test case for converting a DataMessageDTO with an unsupported data point.
 *
 * This test verifies that attempting to convert a DataMessageDTO to a business object (BO)
 * does log the unsupported node but does not throw an exception when a data point is not supported.
 */
TEST_F(DtoToDataMessageIntegrationTest,
       ConvertDataMessageDTONotThrowsExceptionWhenADataPointIsNotSupported) {
    // Define the complete DataMessageDTO
    auto [dto, system_data_points] = assembleValidFunctionArgs();

    // Add an unsupported data point to the DTO, which is not included in the system data points
    dto.data["unsupported_node"] = "value";

    // Output the DTO to be parsed
    std::cout << "DataMessageDTO to parse with missing supported data points:\n"
              << dto << std::endl;

    DataMessage bo = dto_to_bo_.convert(dto, system_data_points);
    std::cout << "DataMessage parsed:\n" << bo << std::endl;

    // Assert: Ensure the unsupported data point is not included in the BO
    ASSERT_EQ(bo.getNodes().size(), 1);  // Only the supported data point should be included
    ASSERT_EQ(bo.getNodes().front().getName(), "Vehicle.node_name");
    ASSERT_EQ(bo.getNodes().front().getValue().value(), "value");
}

/**
 * @brief Test case for converting a StatusMessageDTO to a StatusMessage business object.
 *
 * This test verifies that the conversion from a StatusMessageDTO to a StatusMessage
 * business object is performed correctly. It checks that all fields are accurately
 * transferred from the DTO to the BO.
 */
TEST_F(DtoToDataMessageIntegrationTest, ConvertStatusMessageDtoToBo) {
    // Arrange
    int random_code = RandomUtils::generateRandomInt(100, 999);
    std::string random_message = RandomUtils::generateRandomString(10);
    auto random_timestamps = TestHelper::getSecondsAndNanoseconds();
    auto random_requestId = TestHelper::generateRequestIdTag();

    // Create dynamic StatusMessageDTO
    StatusMessageDTO dto;
    dto.code = random_code;
    dto.message = random_message;
    dto.timestamp.seconds = random_timestamps.first;
    dto.timestamp.nanos = random_timestamps.second;
    dto.requestId = random_requestId;

    std::cout << "StatusMessageDTO to parse:\n" << dto << std::endl;

    // Act
    StatusMessage bo = dto_to_bo_.convert(dto);
    std::cout << "StatusMessage parsed:\n" << bo << std::endl;

    // Assert
    ASSERT_EQ(bo.getCode(), dto.code);
    ASSERT_EQ(bo.getMessage(), dto.message);
    ASSERT_EQ(bo.getRequestId(), dto.requestId);
    ASSERT_EQ(bo.getTimestamp(),
              Helper::convertToTimestamp(dto.timestamp.seconds, dto.timestamp.nanos));
}

/**
 * @brief Test case for converting a StatusMessageDTO with missing required fields.
 *
 * This test verifies that attempting to convert a StatusMessageDTO to a business object (BO)
 * throws an invalid_argument exception when any of the required fields are missing.
 */
TEST_F(DtoToDataMessageIntegrationTest,
       ConvertStatusMessageDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
    std::vector<std::string> required_fields = {"message", "timestamp"};

    for (const auto& field : required_fields) {
        // Define the complete StatusMessageDTO
        StatusMessageDTO dto;
        dto.code = 200;
        dto.message = "OK";
        dto.timestamp.seconds = 1234567890;
        dto.timestamp.nanos = 123456789;

        if (field == "message") {
            dto.message = std::string();
        } else if (field == "timestamp") {
            dto.timestamp.seconds = 0;
            dto.timestamp.nanos = 0;
        }

        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << dto << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_to_bo_.convert(dto), std::invalid_argument);
    }
}