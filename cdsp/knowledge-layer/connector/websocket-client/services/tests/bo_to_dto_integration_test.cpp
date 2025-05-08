#include <gtest/gtest.h>

#include "bo_to_dto.h"
#include "data_points_utils.h"
#include "data_types.h"
#include "get_message.h"
#include "get_message_dto.h"
#include "nlohmann/json.hpp"
#include "node.h"
#include "random_utils.h"
#include "set_message.h"
#include "set_message_dto.h"
#include "subscribe_message.h"
#include "subscribe_message_dto.h"
#include "vin_utils.h"

class BoToDtoIntegrationTest : public ::testing::Test {
   protected:
    BoToDto bo_to_dto_;
};

/**
 * @brief Generates an optional timestamp.
 *
 * This function returns an optional timestamp. It uses a random boolean generator
 * to decide whether to return a valid timestamp or an empty optional.
 *
 * @return std::optional<std::chrono::system_clock::time_point>
 *         An optional containing a randomly generated timestamp if the random
 *         boolean is true, or std::nullopt if false.
 */
std::optional<std::chrono::system_clock::time_point> getOptionalTimestamp(
    const bool required_generated_timestamp = false) {
    if (required_generated_timestamp) {
        return std::optional<std::chrono::system_clock::time_point>(
            RandomUtils::generateRandomTimestamp(2020, 2025, true));
    } else
        return RandomUtils::generateRandomBool()
                   ? std::optional<std::chrono::system_clock::time_point>(
                         RandomUtils::generateRandomTimestamp())
                   : std::nullopt;
}

/**
 * Generates a vector of optional Node objects based on the specified number of points and schema.
 *
 * @param num_points The number of data points to generate.
 * @param schema The schema to be used for naming the nodes.
 * @param add_nodes_values A boolean flag indicating whether to include node values in the Node
 * objects. Defaults to false.
 * @return A vector of Node objects, each representing an optional data point with associated
 * metadata.
 */
std::vector<Node> getOptionalNodes(const int& num_points, const std::string& schema,
                                   const bool add_nodes_values = false,
                                   const bool required_generated_timestamp = false) {
    std::vector<Node> random_bo_nodes;
    auto random_nodes = DataPointsUtils::generateDataPointsWithValues(num_points);
    std::vector<std::string> supported_data_points;

    // Add the supported data points to the list
    for (const auto& [node_name, _] : random_nodes) {
        supported_data_points.push_back(schema + "." + node_name);
    }

    for (const auto& [node_name, value] : random_nodes) {
        Metadata random_metadata(getOptionalTimestamp(), getOptionalTimestamp(true));
        std::optional<std::string> node_value = std::nullopt;
        if (add_nodes_values) {
            node_value = std::optional<std::string>(Helper::variantToString(value));
        }
        Node node(schema + "." + node_name, node_value, random_metadata, supported_data_points);
        random_bo_nodes.push_back(node);
    }
    return random_bo_nodes;
}

/**
 * @brief Test case for converting a GetMessage business object to a GetMessageDTO.
 *
 * This test verifies the conversion of a GetMessage business object to a GetMessageDTO.
 * It checks the integrity and correctness of the conversion process by comparing the fields
 * of the converted GetMessageDTO with the original business object.
 */
TEST_F(BoToDtoIntegrationTest, ConvertGetMessageToDto) {
    // Arrange
    int num_points = RandomUtils::generateRandomInt(0, 5);
    auto random_id = VinUtils::getRandomVinString();
    auto schema = schemaTypeToString(SchemaType::VEHICLE, true);
    auto random_bo_nodes = getOptionalNodes(num_points, schema);

    GetMessage bo(MessageHeader(random_id, SchemaType::VEHICLE), random_bo_nodes);
    std::cout << "GetMessage to convert:\n" << bo << std::endl;

    // Act
    std::vector<GetMessageDTO> dtos = bo_to_dto_.convert(bo);
    std::cout << "Converted JSON GetMessageDTO:\n" << nlohmann::json(dtos) << std::endl;

    // Assert
    int expected_num_points = num_points == 0 ? 1 : num_points;
    ASSERT_EQ(dtos.size(), expected_num_points);
    for (size_t i = 0; i < expected_num_points; ++i) {
        GetMessageDTO dto = dtos[i];
        ASSERT_EQ(dto.type, "get");
        ASSERT_EQ(dto.schema, schemaTypeToString(bo.getHeader().getSchemaType(), true));
        ASSERT_EQ(dto.instance, bo.getHeader().getId());
        ASSERT_EQ(dto.path, num_points == 0
                                ? std::nullopt
                                : std::optional<std::string>(bo.getNodes()[i].getName().substr(
                                      schema.size() + 1)));  // Schema is absent from path
        ASSERT_EQ(dto.format, "flat");                       // Default format for GetMessageDTO
    }
}

/**
 * @brief Test case for converting a SubscribeMessage business object to a vector of
 * SubscribeMessageDTO.
 *
 * This test verifies the conversion of a SubscribeMessage business object to a vector of
 * SubscribeMessageDTO. It checks the integrity and correctness of the conversion process by
 * comparing the fields of the converted SubscribeMessageDTO with the original business object.
 */
TEST_F(BoToDtoIntegrationTest, ConvertSubscribeMessageToDto) {
    // Arrange
    int num_points = RandomUtils::generateRandomInt(0, 5);
    auto random_id = VinUtils::getRandomVinString();
    auto schema = schemaTypeToString(SchemaType::VEHICLE, true);
    auto random_bo_nodes = getOptionalNodes(num_points, schema);

    SubscribeMessage bo(MessageHeader(random_id, SchemaType::VEHICLE), random_bo_nodes);
    std::cout << "SubscribeMessage to convert:\n" << bo << std::endl;

    // Act
    std::vector<SubscribeMessageDTO> dtos = bo_to_dto_.convert(bo);
    std::cout << "Converted JSON SubscribeMessageDTO:\n" << nlohmann::json(dtos) << std::endl;

    // Assert
    int expected_num_points = num_points == 0 ? 1 : num_points;
    ASSERT_EQ(dtos.size(), expected_num_points);
    for (size_t i = 0; i < expected_num_points; ++i) {
        SubscribeMessageDTO dto = dtos[i];
        ASSERT_EQ(dto.type, "subscribe");
        ASSERT_EQ(dto.schema, schemaTypeToString(bo.getHeader().getSchemaType(), true));
        ASSERT_EQ(dto.instance, bo.getHeader().getId());
        ASSERT_EQ(dto.path, num_points == 0
                                ? std::nullopt
                                : std::optional<std::string>(bo.getNodes()[i].getName()));
        ASSERT_EQ(dto.format, "flat");  // Default format for SubscribeMessageDTO
    }
}

/**
 * @brief Test case for converting a SetMessage business object to a SetMessageDTO.
 *
 * This test verifies the conversion of a SetMessage business object to a SetMessageDTO.
 * It checks the integrity and correctness of the conversion process by comparing the fields
 * of the converted SetMessageDTO with the original business object.
 */
TEST_F(BoToDtoIntegrationTest, ConvertSetMessageToDto) {
    // Arrange
    int num_points = RandomUtils::generateRandomInt(1, 5);
    auto random_id = VinUtils::getRandomVinString();
    auto schema = schemaTypeToString(SchemaType::VEHICLE, true);
    auto random_bo_nodes = getOptionalNodes(num_points, schema, true, true);

    SetMessage bo(MessageHeader(random_id, SchemaType::VEHICLE), random_bo_nodes);
    std::cout << "SetMessage to convert:\n" << bo << std::endl;

    // Act
    SetMessageDTO dto = bo_to_dto_.convert(bo);
    std::cout << "Converted JSON SetMessageDTO:\n" << nlohmann::json(dto) << std::endl;

    // Assert
    ASSERT_EQ(dto.schema, schemaTypeToString(bo.getHeader().getSchemaType(), true));
    ASSERT_EQ(dto.instance, bo.getHeader().getId());
    ASSERT_EQ(dto.data.size(), bo.getNodes().size());
    const auto& nodes = bo.getNodes();
    for (size_t i = 0; i < dto.data.size(); ++i) {
        const DataDTO& data = dto.data[i];
        const Node& node = nodes[i];

        ASSERT_EQ(data.name, node.getName()) << "Mismatch in node name at index " << i;
        auto [seconds, nanos] =
            Helper::getSecondsAndNanosecondsSinceEpoch(node.getMetadata().getGenerated().value());

        MetadataDTO::NodeMetadata metadata{.generated = {nanos, seconds}};
        ASSERT_EQ(dto.metadata.nodes[data.name].generated.seconds, seconds)
            << "Mismatch in generated seconds for node: " << node.getName();

        ASSERT_EQ(dto.metadata.nodes[data.name].generated.nanos, nanos)
            << "Mismatch in generated nanoseconds for node: " << node.getName();
        ASSERT_EQ(dto.metadata.nodes[data.name].received.seconds, 0)  // This should not be set
            << "Mismatch in received seconds for node: " << node.getName();
        ASSERT_EQ(dto.metadata.nodes[data.name].received.nanos, 0)  // This should not be set
            << "Mismatch in received nanoseconds for node: " << node.getName();
        if (node.getValue()) {
            try {
                auto expected_json = nlohmann::json::parse(*node.getValue());
                ASSERT_EQ(data.value, expected_json)
                    << "Parsed value mismatch for node: " << node.getName();
            } catch (const std::exception&) {
                ASSERT_TRUE(data.value.is_string()) << "Expected fallback to string value";
                ASSERT_EQ(data.value, *node.getValue())
                    << "Fallback value mismatch for node: " << node.getName();
            }
        } else {
            ASSERT_TRUE(data.value.is_string()) << "Expected empty string for nullopt value";
            ASSERT_EQ(data.value, "") << "Expected empty string for node with no value";
        }
    }
}