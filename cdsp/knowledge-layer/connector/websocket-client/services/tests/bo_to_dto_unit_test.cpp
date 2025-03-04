#include <gtest/gtest.h>

#include "bo_to_dto.h"
#include "data_points_utils.h"
#include "data_types.h"
#include "get_message.h"
#include "get_message_dto.h"
#include "node.h"
#include "random_utils.h"
#include "vin_utils.h"

class BoToDtoUnitTest : public ::testing::Test {
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
std::optional<std::chrono::system_clock::time_point> getOptionalTimestamp() {
    return RandomUtils::generateRandomBool() ? std::optional<std::chrono::system_clock::time_point>(
                                                   RandomUtils::generateRandomTimestamp())
                                             : std::nullopt;
}

/**
 * @brief Retrieves an optional MessageStructureFormat.
 *
 * This function generates a random boolean value to decide whether to return
 * a MessageStructureFormat or an empty optional. If the boolean is true, it
 * selects a random MessageStructureFormat from a predefined list of formats
 * (FLAT, NESTED, LEAF) and returns it wrapped in an std::optional. If the
 * boolean is false, it returns an empty std::optional.
 *
 * @return std::optional<MessageStructureFormat> An optional containing a
 * randomly selected MessageStructureFormat or std::nullopt.
 */
std::optional<MessageStructureFormat> getOptionalMessageStructureFormat() {
    std::vector<MessageStructureFormat> formats = {
        MessageStructureFormat::FLAT, MessageStructureFormat::NESTED, MessageStructureFormat::LEAF};

    return RandomUtils::generateRandomBool() ? std::optional<MessageStructureFormat>(
                                                   formats[RandomUtils::generateRandomInt(0, 2)])
                                             : std::nullopt;
}

/**
 * Generates a vector of optional nodes with random metadata.
 *
 * This function creates a vector of `Node` objects, each initialized with a
 * name from a randomly generated set of data points and optional metadata.
 * The metadata includes two optional timestamps.
 *
 * @param num_points The number of data points to generate.
 * @return A vector of `Node` objects with optional metadata.
 */
std::vector<Node> getOptionalNodes(const int& num_points, const std::string& schema) {
    std::vector<Node> random_bo_nodes;
    auto random_nodes = DataPointsUtils::generateDataPointsWithValues(num_points);
    std::vector<std::string> supported_data_points;

    // Add the supported data points to the list
    for (const auto& [node_name, _] : random_nodes) {
        supported_data_points.push_back(schema + "." + node_name);
    }

    for (const auto& [node_name, _] : random_nodes) {
        Metadata random_metadata(getOptionalTimestamp(), getOptionalTimestamp());
        Node node(schema + "." + node_name, std::nullopt, random_metadata,
                  supported_data_points);  // Nodes are named as e.g. "Vehicle.<node_name>"
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
TEST_F(BoToDtoUnitTest, ConvertGetMessageToDto) {
    // Arrange
    int num_points = RandomUtils::generateRandomInt(0, 5);
    auto random_id = VinUtils::getRandomVinString();
    auto random_structure_format = getOptionalMessageStructureFormat();
    auto schema = schemaTypeToString(SchemaType::VEHICLE, true);
    auto random_bo_nodes = getOptionalNodes(num_points, schema);

    GetMessage bo(MessageHeader(random_id, SchemaType::VEHICLE), random_bo_nodes);
    std::cout << "GetMessage to convert:\n" << bo << std::endl;

    // Act
    std::vector<GetMessageDTO> dtos = bo_to_dto_.convert(bo, random_structure_format);
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
        ASSERT_EQ(dto.format, random_structure_format
                                  ? std::optional<std::string>(
                                        stringToMessageStructureFormat(*random_structure_format))
                                  : std::nullopt);
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
TEST_F(BoToDtoUnitTest, ConvertSubscribeMessageToDto) {
    // Arrange
    int num_points = RandomUtils::generateRandomInt(0, 5);
    auto random_id = VinUtils::getRandomVinString();
    auto random_structure_format = getOptionalMessageStructureFormat();
    auto schema = schemaTypeToString(SchemaType::VEHICLE, true);
    auto random_bo_nodes = getOptionalNodes(num_points, schema);

    SubscribeMessage bo(MessageHeader(random_id, SchemaType::VEHICLE), random_bo_nodes);
    std::cout << "SubscribeMessage to convert:\n" << bo << std::endl;

    // Act
    std::vector<SubscribeMessageDTO> dtos = bo_to_dto_.convert(bo, random_structure_format);
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
        ASSERT_EQ(dto.format, random_structure_format
                                  ? std::optional<std::string>(
                                        stringToMessageStructureFormat(*random_structure_format))
                                  : std::nullopt);
    }
}
