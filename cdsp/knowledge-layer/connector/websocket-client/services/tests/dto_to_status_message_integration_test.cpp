#include <gtest/gtest.h>

#include <utility>

#include "dto_to_bo.h"
#include "helper.h"
#include "random_utils.h"
#include "test_helper.h"

class DtoToStatusMessageIntegrationTest : public ::testing::Test {
   protected:
    DtoToBo dto_to_bo_;
    const SchemaType SCHEMA_TYPE = SchemaType::VEHICLE;
};

/**
 * @brief Test case for converting a StatusMessageDTO to a StatusMessage business object.
 *
 * This test verifies that the conversion from a StatusMessageDTO to a StatusMessage
 * business object is performed correctly. It checks that all fields are accurately
 * transferred from the DTO to the BO.
 */
TEST_F(DtoToStatusMessageIntegrationTest, ConvertStatusMessageDtoToBo) {
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
TEST_F(DtoToStatusMessageIntegrationTest,
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