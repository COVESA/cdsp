#include <gtest/gtest.h>

#include <iostream>

#include "bo_service.h"
#include "data_points_utils.h"
#include "random_utils.h"
class BoServiceIntegrationTest : public ::testing::Test {
   protected:
    BoService bo_service_;
    const std::string OBJECT_ID = "object_id";
    const SchemaType SCHEMA_TYPE = SchemaType::VEHICLE;
};

/**
 * @brief Test case for creating a SubscribeMessage.
 *
 * This test verifies that the SubscribeMessage is created correctly with the provided
 * object ID and schema type.
 */
TEST_F(BoServiceIntegrationTest, CreateSubscribeMessage) {
    // Act
    SubscribeMessage subscribe_message = bo_service_.createSubscribeMessage(OBJECT_ID, SCHEMA_TYPE);

    std::cout << "SubscribeMessage created:\n" << subscribe_message << std::endl;

    // Assert
    ASSERT_EQ(subscribe_message.getHeader().getId(), OBJECT_ID) << "Object ID mismatch";
    ASSERT_EQ(subscribe_message.getHeader().getSchemaType(), SCHEMA_TYPE) << "Schema type mismatch";
    // TODO: Change when required, at this point, the subscribe message subscribes to all nodes in
    // the schema collection.
    ASSERT_TRUE(subscribe_message.getNodes().empty()) << "Nodes should be empty";
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
    for (int i = 0; i < RandomUtils::generateRandomInt(1, 5); ++i) {
        list_data_points.push_back(
            DataPointsUtils::generateRandomKey(RandomUtils::generateRandomInt(1, 5)));
    }

    // Act
    GetMessage get_message = bo_service_.createGetMessage(OBJECT_ID, SCHEMA_TYPE, list_data_points);

    std::cout << "List of data points:\n";
    for (const auto& data_point : list_data_points) {
        std::cout << data_point << std::endl;
    }

    std::cout << "GetMessage created:\n" << get_message << std::endl;

    // Assert
    ASSERT_EQ(get_message.getHeader().getId(), OBJECT_ID) << "Object ID mismatch";
    ASSERT_EQ(get_message.getHeader().getSchemaType(), SCHEMA_TYPE) << "Schema type mismatch";
    ASSERT_EQ(get_message.getNodes().size(), list_data_points.size()) << "Number of nodes mismatch";
    for (size_t i = 0; i < list_data_points.size(); ++i) {
        ASSERT_EQ(get_message.getNodes()[i].getName(), list_data_points[i])
            << "Node name mismatch at index " << i;
        ASSERT_FALSE(get_message.getNodes()[i].getValue().has_value())
            << "Any value should be included in the Get Message at index " << i;
        ASSERT_FALSE(get_message.getNodes()[i].getMetadata().getGenerated().has_value())
            << "Generated time should be empty at index " << i;
        // This uses the default value of the Metadata class, which is set if the value is not
        // provided
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
    for (int i = 0; i < RandomUtils::generateRandomInt(1, 5); ++i) {
        std::unordered_map<std::string, std::variant<std::string, int, double, float, bool>>
            data_points =
                DataPointsUtils::generateDataPointsWithValues(RandomUtils::generateRandomInt(1, 5));

        nlohmann::json vehicle_data;
        for (const auto& [key, value] : data_points) {
            vehicle_data[key] = Helper::variantToString(value);
        }

        json_data.push_back({{"Vehicle", vehicle_data}});
    }

    // Act
    std::vector<SetMessage> set_messages = bo_service_.createSetMessage(object_id_map, json_data);

    // Assert
    ASSERT_EQ(set_messages.size(), json_data.size()) << "Number of SetMessages mismatch";
    for (size_t i = 0; i < json_data.size(); ++i) {
        ASSERT_EQ(set_messages[i].getHeader().getId(), OBJECT_ID)
            << "Object ID mismatch at index " << i;
        ASSERT_EQ(set_messages[i].getHeader().getSchemaType(), SCHEMA_TYPE)
            << "Schema type mismatch at index " << i;
        std::cout << "Received JSON data[" << i << "]:\n"
                  << json_data[i].dump(4) << "\n"
                  << std::endl;
        std::cout << "Generated SetMessage[" << i << "]:\n" << set_messages[i] << "\n" << std::endl;

        ASSERT_EQ(set_messages[i].getNodes().size(), json_data[i]["Vehicle"].size())
            << "Node size mismatch at index " << i;
        const auto& vehicle_data = json_data[i]["Vehicle"];
        size_t j = 0;
        for (const auto& [key, value] : vehicle_data.items()) {
            ASSERT_EQ(set_messages[i].getNodes()[j].getName(), key)
                << "Node name mismatch at index " << i << ", node " << j;
            ASSERT_EQ(set_messages[i].getNodes()[j].getValue().value(), value.dump())
                << "Node value mismatch at index " << i
                << ", SetMessage value: " << set_messages[i].getNodes()[j].getValue().value()
                << ", JSON: " << value << std::endl;
            ASSERT_TRUE(set_messages[i]
                            .getNodes()[j]
                            .getMetadata()
                            .getGenerated()
                            .value()
                            .time_since_epoch()
                            .count() > 0)
                << "Generated time should not be empty at index " << i << ", node " << j;
            // This uses the default value of the Metadata class, which is set if the value
            // is not provided
            ASSERT_TRUE(set_messages[i]
                            .getNodes()[j]
                            .getMetadata()
                            .getReceived()
                            .time_since_epoch()
                            .count() > 0)
                << "Received time should not be empty at index " << i << ", node " << j;
            // }
            ++j;
        }
    }
}