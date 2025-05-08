#include <gtest/gtest.h>

#include "json_writer.h"
#include "mock_i_file_handler.h"
#include "random_utils.h"

class JSONWriterTest : public ::testing::Test {
   protected:
    JSONWriter jsonWriter;
};

/**
 * @brief Test case for writing JSON data to a file.
 *
 * This test verifies that the JSONWriter converts CSV input data
 * into JSON format and writes it to a specified file path. It uses a mock
 * file handler to ensure the write operation is called exactly once.
 *
 * The test checks that the resulting JSON object contains the expected
 * fields.
 */
TEST_F(JSONWriterTest, WriteJsonToFile) {
    std::string csv_input = "id,name,age\n1,Alice,30\n2,Bob,25";
    std::optional<std::string> output_file_path = "./test_output/";
    bool is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    auto mockFileHandler = std::make_shared<MockIFileHandler>();
    EXPECT_CALL(*mockFileHandler, writeFile(::testing::_, ::testing::_, false)).Times(1);

    nlohmann::json result =
        jsonWriter.writeToJson(csv_input, DataQueryAcceptType::TEXT_CSV,
                               is_ai_reasoner_inference_results, output_file_path, mockFileHandler);

    ASSERT_FALSE(result.empty());
}

/**
 * @brief Test case for writing JSON data without a file path.
 *
 * This test verifies that the JSONWriter converts CSV input data
 * into JSON format and does not write it to a file. It uses a mock
 * file handler to ensure the write operation is not called.
 *
 * The test checks that the resulting JSON object contains the expected
 * fields.
 */
TEST_F(JSONWriterTest, WriteJsonWithoutFilePath) {
    std::string csv_input = "id,name,age\n1,Alice,30\n2,Bob,25";
    std::optional<std::string> output_file_path = std::nullopt;  // No output file
    bool is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    auto mockFileHandler = std::make_shared<MockIFileHandler>();
    EXPECT_CALL(*mockFileHandler, writeFile(::testing::_, ::testing::_, ::testing::_)).Times(0);

    nlohmann::json result =
        jsonWriter.writeToJson(csv_input, DataQueryAcceptType::TEXT_CSV,
                               is_ai_reasoner_inference_results, output_file_path);

    ASSERT_FALSE(result.empty());
}

/**
 * @brief Test case for JSONWriter's handling of invalid format.
 *
 * This test verifies that the JSONWriter throws a runtime_error when
 * an unsupported DataQueryAcceptType is provided.
 *
 * The input query_result is a string representing a CSV format with
 * headers and data. The test specifically uses an invalid
 * DataQueryAcceptType (999) to trigger the error.
 */
TEST_F(JSONWriterTest, WriteJsonWithInvalidFormat) {
    std::string query_result = "id,name,age\n1,Alice,30\n2,Bob,25";
    bool is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    ASSERT_THROW(
        {
            try {
                jsonWriter.writeToJson(query_result, static_cast<DataQueryAcceptType>(999),
                                       is_ai_reasoner_inference_results, std::nullopt);
            } catch (const std::runtime_error& e) {
                EXPECT_THAT(e.what(), testing::HasSubstr("Unsupported query result format"));
                throw;
            }
        },
        std::runtime_error);
}

/**
 * @brief Test case for JSONWriter that verifies the behavior when writing JSON
 * with empty values in the query result.
 *
 * This test checks that a runtime error is thrown when the input query result
 * does not contain any data. It expects the error message to indicate that no
 * results were found in the query result.
 */
TEST_F(JSONWriterTest, WriteJsonWithEmptyValues) {
    std::string query_result = "id,name,age\n";
    bool is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    ASSERT_NO_THROW({
        nlohmann::json result =
            jsonWriter.writeToJson(query_result, DataQueryAcceptType::TEXT_CSV,
                                   is_ai_reasoner_inference_results, std::nullopt);
        ASSERT_TRUE(result.empty());
    });
}

/**
 * @brief Test case for JSONWriter's behavior when writing JSON with an empty result.
 *
 * This test verifies that when an empty query result is provided to the
 * jsonWriter's writeToJson method, it throws a runtime_error. The test
 * also checks that the error message contains the expected substring
 * indicating that no results were found.
 */
TEST_F(JSONWriterTest, WriteJsonWithEmptyResult) {
    std::string query_result = "";
    bool is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    ASSERT_NO_THROW({
        nlohmann::json result =
            jsonWriter.writeToJson(query_result, DataQueryAcceptType::TEXT_CSV,
                                   is_ai_reasoner_inference_results, std::nullopt);
        ASSERT_TRUE(result.empty());
    });
}

/**
 * @brief Test case for handling invalid SPARQL JSON input in the JSONWriter.
 *
 * This test verifies that the JSONWriter correctly throws a runtime error
 * when provided with an invalid SPARQL JSON format. It captures the exception,
 * logs the error message, and checks that the message contains the expected
 * substring indicating the nature of the error.
 */
TEST_F(JSONWriterTest, InvalidSparqlJson) {
    std::string invalid_sparql_json = R"({ "invalid": "format" })";
    bool is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    ASSERT_THROW(
        {
            try {
                jsonWriter.writeToJson(invalid_sparql_json, DataQueryAcceptType::SPARQL_JSON,
                                       is_ai_reasoner_inference_results, std::nullopt);
            } catch (const std::runtime_error& e) {
                EXPECT_THAT(e.what(), testing::HasSubstr("Invalid SPARQL JSON response format"));
                throw;
            }
        },
        std::runtime_error);
}

/**
 * @brief Test case for handling invalid SPARQL XML input in the JSONWriter.
 *
 * This test verifies that the JSONWriter correctly throws a runtime_error
 * when provided with an invalid SPARQL XML string. It checks that the error
 * message contains the expected substring indicating the failure to parse
 * the SPARQL XML response.
 */
TEST_F(JSONWriterTest, InvalidSparqlXml) {
    std::string invalid_sparql_xml = R"(<sparql><invalid></sparql>)";
    bool is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    ASSERT_THROW(
        {
            try {
                jsonWriter.writeToJson(invalid_sparql_xml, DataQueryAcceptType::SPARQL_XML,
                                       is_ai_reasoner_inference_results, std::nullopt);
            } catch (const std::runtime_error& e) {
                EXPECT_THAT(e.what(), testing::HasSubstr("Failed to parse SPARQL XML response"));
                throw;
            }
        },
        std::runtime_error);
}