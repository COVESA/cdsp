#include <gtest/gtest.h>

#include <iostream>

#include "bo_service.h"
#include "data_points_utils.h"
#include "random_utils.h"
class BoServiceIntegrationTest : public ::testing::Test {
    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
   protected:
    const std::string OBJECT_ID = "object_id";
    const SchemaType SCHEMA_TYPE = SchemaType::VEHICLE;
    static constexpr int MAX_RANDOM_SIZE = 5;
    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)
};

/**
 * @brief Test case for creating a SubscribeMessage.
 *
 * This test verifies that the SubscribeMessage is created correctly with the
 * provided object ID and schema type.
 */
TEST_F(BoServiceIntegrationTest, CreateSubscribeMessage) {
    // Arrange
    std::vector<Node> nodes;
    int num_points = RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE);
    nodes.reserve(num_points);
    for (int i = 0; i < num_points; ++i) {
        Node node{
            DataPointsUtils::generateRandomKey(RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE)),
            std::nullopt, Metadata()};
        nodes.emplace_back(node);
    }

    // Act
    SubscribeMessage subscribe_message =
        BoService::createSubscribeMessage(OBJECT_ID, SCHEMA_TYPE, nodes);

    std::cout << "SubscribeMessage created:\n" << subscribe_message << "\n";

    // Assert
    ASSERT_EQ(subscribe_message.getHeader().getInstance(), OBJECT_ID) << "Object ID mismatch";
    ASSERT_EQ(subscribe_message.getHeader().getSchemaType(), SCHEMA_TYPE) << "Schema type mismatch";
    ASSERT_EQ(subscribe_message.getNodes().size(), nodes.size()) << "Number of nodes mismatch";
    for (size_t i = 0; i < nodes.size(); ++i) {
        ASSERT_EQ(subscribe_message.getNodes()[i].getName(), nodes[i].getName())
            << "Node name mismatch at index " << i;
        ASSERT_FALSE(subscribe_message.getNodes()[i].getMetadata().getGenerated().has_value())
            << "Generated time should be empty at index " << i;
        // This uses the default value of the Metadata class, which is set if the
        // value is not provided
        ASSERT_TRUE(
            subscribe_message.getNodes()[i].getMetadata().getReceived().time_since_epoch().count() >
            0)
            << "Received time should not be empty at index " << i;
    }
}

/**
 * @brief Test case for creating an UnsubscribeMessage.
 *
 * This test verifies that the UnsubscribeMessage is created correctly with the
 * provided object ID, schema type, and nodes.
 */
TEST_F(BoServiceIntegrationTest, CreateUnsubscribeMessage) {
    // Arrange
    std::vector<Node> nodes;
    int num_points = RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE);
    nodes.reserve(num_points);
    for (int i = 0; i < num_points; ++i) {
        Node node{
            DataPointsUtils::generateRandomKey(RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE)),
            std::nullopt, Metadata()};
        nodes.emplace_back(node);
    }
    // Act
    UnsubscribeMessage unsubscribe_message =
        BoService::createUnsubscribeMessage(OBJECT_ID, SCHEMA_TYPE, nodes);
    std::cout << "UnsubscribeMessage created:\n" << unsubscribe_message << "\n";
    // Assert
    ASSERT_EQ(unsubscribe_message.getHeader().getInstance(), OBJECT_ID) << "Object ID mismatch";
    ASSERT_EQ(unsubscribe_message.getHeader().getSchemaType(), SCHEMA_TYPE)
        << "Schema type mismatch";
    ASSERT_EQ(unsubscribe_message.getNodes().size(), nodes.size()) << "Number of nodes mismatch";
    for (size_t i = 0; i < nodes.size(); ++i) {
        ASSERT_EQ(unsubscribe_message.getNodes()[i].getName(), nodes[i].getName())
            << "Node name mismatch at index " << i;
        ASSERT_FALSE(unsubscribe_message.getNodes()[i].getMetadata().getGenerated().has_value())
            << "Generated time should be empty at index " << i;
        // This uses the default value of the Metadata class, which is set if the
        // value is not provided
        ASSERT_TRUE(unsubscribe_message.getNodes()[i]
                        .getMetadata()
                        .getReceived()
                        .time_since_epoch()
                        .count() > 0)
            << "Received time should not be empty at index " << i;
    }
}

/**
 * @brief Test case for creating a GetMessage.
 *
 * This test verifies that the GetMessage is created correctly with the provided
 * object ID, schema type, and list of data points.
 */
TEST_F(BoServiceIntegrationTest, CreateGetMessage) {
    // Arrange
    std::vector<std::string> list_data_points;
    int random_size = RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE);
    list_data_points.reserve(random_size);
    for (int i = 0; i < random_size; ++i) {
        list_data_points.push_back(
            DataPointsUtils::generateRandomKey(RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE)));
    }

    // Act
    GetMessage get_message = BoService::createGetMessage(OBJECT_ID, SCHEMA_TYPE, list_data_points);

    std::cout << "List of data points:\n";
    for (const auto &data_point : list_data_points) {
        std::cout << data_point << "\n";
    }

    std::cout << "GetMessage created:\n" << get_message << "\n";

    // Assert
    ASSERT_EQ(get_message.getHeader().getInstance(), OBJECT_ID) << "Object ID mismatch";
    ASSERT_EQ(get_message.getHeader().getSchemaType(), SCHEMA_TYPE) << "Schema type mismatch";
    ASSERT_EQ(get_message.getNodes().size(), list_data_points.size()) << "Number of nodes mismatch";
    for (size_t i = 0; i < list_data_points.size(); ++i) {
        ASSERT_EQ(get_message.getNodes()[i].getName(), list_data_points[i])
            << "Node name mismatch at index " << i;
        ASSERT_FALSE(get_message.getNodes()[i].getValue().has_value())
            << "Any value should be included in the Get Message at index " << i;
        ASSERT_FALSE(get_message.getNodes()[i].getMetadata().getGenerated().has_value())
            << "Generated time should be empty at index " << i;
        // This uses the default value of the Metadata class, which is set if the
        // value is not provided
        ASSERT_TRUE(
            get_message.getNodes()[i].getMetadata().getReceived().time_since_epoch().count() > 0)
            << "Received time should not be empty at index " << i;
    }
}

/**
 * @brief Test case for creating a SetMessage.
 *
 * This test verifies that the SetMessage is created correctly with the provided
 * object ID map and JSON data.
 */
TEST_F(BoServiceIntegrationTest, CreateSetMessage) {
    // Arrange
    std::map<SchemaType, std::string> object_id_map;
    object_id_map[SCHEMA_TYPE] = OBJECT_ID;

    nlohmann::json json_data = nlohmann::json::array();
    for (int i = 0; i < RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE); ++i) {
        std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>
            data_points = DataPointsUtils::generateDataPointsWithValues(
                RandomUtils::generateRandomInt(1, MAX_RANDOM_SIZE));

        nlohmann::json vehicle_data;
        for (const auto &[key, value] : data_points) {
            vehicle_data[key] = Helper::variantToString(value);
        }

        json_data.push_back({{"Vehicle", vehicle_data}});
    }

    // Populate the origin system name if it exists
    const std::string origin_system_name = "test_reasoner";

    // Act
    std::vector<SetMessage> set_messages =
        BoService::createSetMessage(object_id_map, json_data, origin_system_name);

    // Assert
    ASSERT_EQ(set_messages.size(), json_data.size()) << "Number of SetMessages mismatch";
    for (size_t i = 0; i < json_data.size(); ++i) {
        ASSERT_EQ(set_messages[i].getHeader().getInstance(), OBJECT_ID)
            << "Object ID mismatch at index " << i;
        ASSERT_EQ(set_messages[i].getHeader().getSchemaType(), SCHEMA_TYPE)
            << "Schema type mismatch at index " << i;
        std::cout << "Received JSON data[" << i << "]:\n"
                  << json_data[i].dump(4) << "\n"
                  << "\n";
        std::cout << "Generated SetMessage[" << i << "]:\n"
                  << set_messages[i] << "\n"
                  << "\n";

        ASSERT_EQ(set_messages[i].getNodes().size(), json_data[i]["Vehicle"].size())
            << "Node size mismatch at index " << i;
        const auto &vehicle_data = json_data[i]["Vehicle"];
        size_t index = 0;
        for (const auto &[key, value] : vehicle_data.items()) {
            ASSERT_EQ(set_messages[i].getNodes()[index].getName(), key)
                << "Node name mismatch at index " << i << ", node " << index;
            ASSERT_EQ(set_messages[i].getNodes()[index].getValue().value(), value.dump())
                << "Node value mismatch at index " << i
                << ", SetMessage value: " << set_messages[i].getNodes()[index].getValue().value()
                << ", JSON: " << value << "\n";
            ASSERT_TRUE(set_messages[i]
                            .getNodes()[index]
                            .getMetadata()
                            .getGenerated()
                            .value()
                            .time_since_epoch()
                            .count() > 0)
                << "Generated time should not be empty at index " << i << ", node " << index;
            // This uses the default value of the Metadata class, which is set if the
            // value is not provided
            ASSERT_TRUE(set_messages[i]
                            .getNodes()[index]
                            .getMetadata()
                            .getReceived()
                            .time_since_epoch()
                            .count() > 0)
                << "Received time should not be empty at index " << i << ", node " << index;
            // }
            ++index;
        }
    }
}