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

class ModelConfigDtoServiceUnitTest : public ::testing::Test {
   protected:
    DtoService dto_service_;
};

nlohmann::json generateValidStatusMessageJson() {
    return {{"code", 200},
            {"message", "OK"},
            {"timestamp", {{"seconds", 1234567890}, {"nanos", 123456789}}}};
}

// Test for parsing a Status Message JSON

/**
 * @brief Test case for parsing a valid StatusMessage JSON.
 *
 * This test verifies the parsing of a valid StatusMessage JSON object into a StatusMessageDTO.
 * It checks the integrity and correctness of the parsing process by comparing the fields
 * of the parsed StatusMessageDTO with the original JSON object.
 */
TEST_F(ModelConfigDtoServiceUnitTest, ParseStatusMessage) {
    // Arrange
    int random_code = RandomUtils::generateRandomInt(100, 999);
    std::string random_message = RandomUtils::generateRandomString(10);
    auto random_timestamps = TestHelper::getSecondsAndNanoseconds();
    auto random_requestId = TestHelper::generateRequestIdTag();

    nlohmann::json json_message = {
        {"code", random_code},
        {"message", random_message},
        {"timestamp", {{"seconds", random_timestamps.first}, {"nanos", random_timestamps.second}}}};
    if (random_requestId.has_value()) {
        json_message["requestId"] = random_requestId.value();
    }

    std::cout << "Incoming message: \n" << json_message.dump(4) << std::endl;

    // Act
    StatusMessageDTO dto = dto_service_.parseStatusJsonToDto(json_message);
    std::cout << "Parsed StatusMessageDTO: \n" << dto << std::endl;

    // Assert
    ASSERT_EQ(dto.code, random_code);
    ASSERT_EQ(dto.message, random_message);
    ASSERT_EQ(dto.requestId, random_requestId);
    ASSERT_EQ(dto.timestamp.seconds, random_timestamps.first);
    ASSERT_EQ(dto.timestamp.nanos, random_timestamps.second);
}

/**
 * @brief Unit test for the DtoService class to verify exception handling for missing required
 * fields for the status message.
 *
 * This test case checks that the `parseStatusJsonToDto` method of the `dto_service_` object
 * throws an `std::invalid_argument` exception when any of the required fields are missing
 * from the JSON message. The required fields are "code", "message", and "timestamp".
 *
 * The test iterates over each required field, removes it from a valid JSON message,
 * and asserts that an exception is thrown when the modified message is parsed.
 */
TEST_F(ModelConfigDtoServiceUnitTest, ParseStatusDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
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
        StatusMessageDTO expected_dto;
        ASSERT_NO_THROW({
            StatusMessageDTO result = dto_service_.parseStatusJsonToDto(test_message);
            ASSERT_EQ(result.code, 0);
            ASSERT_TRUE(result.message.empty());
            ASSERT_EQ(result.timestamp.seconds, 0);
            ASSERT_EQ(result.timestamp.nanos, 0);
        });
    }
}

/**
 * @brief Unit test for the DtoService class to verify exception handling for incomplete timestamps.
 *
 * This test case checks that the `parseStatusJsonToDto` method of the `dto_service_` object
 * throws an `std::invalid_argument` exception when any of the required fields in the timestamp
 * are missing from the JSON message. The required fields within the "timestamp" are "seconds" and
 * "nanos".
 *
 * The test iterates over each required field, removes it from a valid JSON message's timestamp,
 * and asserts that an exception is thrown when the modified message is parsed.
 */
TEST_F(ModelConfigDtoServiceUnitTest, ParseStatusDtoThrowsExceptionWhenTimestampIsIncomplete) {
    // Define the complete JSON message
    nlohmann::json json_message = generateValidStatusMessageJson();

    // List of required fields in the timestamp
    std::vector<std::string> required_fields = {"seconds", "nanos"};

    // Iterate over each required field
    for (const auto& field : required_fields) {
        nlohmann::json test_message = json_message;
        test_message["timestamp"].erase(field);  // Remove the required field

        // Output the current test case details
        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << test_message.dump(4) << std::endl;

        StatusMessageDTO expected_dto;
        ASSERT_NO_THROW({
            StatusMessageDTO result = dto_service_.parseStatusJsonToDto(test_message);
            ASSERT_EQ(result.code, 0);
            ASSERT_TRUE(result.message.empty());
            ASSERT_EQ(result.timestamp.seconds, 0);
            ASSERT_EQ(result.timestamp.nanos, 0);
        });
    }
}
