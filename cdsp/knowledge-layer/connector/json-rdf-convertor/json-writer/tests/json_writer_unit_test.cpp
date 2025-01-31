#include <gtest/gtest.h>

#include "json_writer.h"
#include "mock_i_file_handler.h"

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
 * fields "created_at" and "results", and that the "results" field is not empty.
 */
TEST_F(JSONWriterTest, WriteJsonToFile) {
    std::string csv_input = "id,name,age\n1,Alice,30\n2,Bob,25";
    std::optional<std::string> output_file_path = "./test_output/";

    auto mockFileHandler = std::make_shared<MockIFileHandler>();
    EXPECT_CALL(*mockFileHandler, writeFile(::testing::_, ::testing::_, false)).Times(1);

    nlohmann::json result = jsonWriter.writeToJson(csv_input, DataQueryAcceptType::TEXT_CSV,
                                                   output_file_path, mockFileHandler);

    ASSERT_TRUE(result.contains("created_at"));
    ASSERT_TRUE(result.contains("results"));
    ASSERT_FALSE(result["results"].empty());
}

/**
 * @brief Test case for writing JSON data without a file path.
 *
 * This test verifies that the JSONWriter converts CSV input data
 * into JSON format and does not write it to a file. It uses a mock
 * file handler to ensure the write operation is not called.
 *
 * The test checks that the resulting JSON object contains the expected
 * fields "created_at" and "results", and that the "results" field is not empty.
 */
TEST_F(JSONWriterTest, WriteJsonWithoutFilePath) {
    std::string csv_input = "id,name,age\n1,Alice,30\n2,Bob,25";
    std::optional<std::string> output_file_path = std::nullopt;  // No output file

    auto mockFileHandler = std::make_shared<MockIFileHandler>();
    EXPECT_CALL(*mockFileHandler, writeFile(::testing::_, ::testing::_, ::testing::_)).Times(0);

    nlohmann::json result =
        jsonWriter.writeToJson(csv_input, DataQueryAcceptType::TEXT_CSV, output_file_path);

    ASSERT_TRUE(result.contains("created_at"));
    ASSERT_TRUE(result.contains("results"));
    ASSERT_FALSE(result["results"].empty());
}

/**
 * @brief Test case for writing JSON data with an invalid format.
 *
 * This test verifies that the JSONWriter throws a runtime error when
 * attempting to write JSON data with an unsupported format type.
 */
TEST_F(JSONWriterTest, WriteJsonWithInvalidFormat) {
    std::string query_result = "id,name,age\n1,Alice,30\n2,Bob,25";

    EXPECT_THROW(
        jsonWriter.writeToJson(query_result, static_cast<DataQueryAcceptType>(999), std::nullopt),
        std::runtime_error);
}

/**
 * @brief Test case for handling invalid SPARQL JSON input.
 *
 * This test verifies that the JSONWriter throws a runtime error
 * when attempting to write an invalid SPARQL JSON format. The test
 * uses an invalid JSON string to simulate erroneous input and expects
 * the writeToJson method to throw a std::runtime_error.
 */
TEST_F(JSONWriterTest, InvalidSparqlJson) {
    std::string invalid_sparql_json = R"({ "invalid": "format" })";

    EXPECT_THROW(
        jsonWriter.writeToJson(invalid_sparql_json, DataQueryAcceptType::SPARQL_JSON, std::nullopt),
        std::runtime_error);
}

/**
 * @brief Test case for handling invalid SPARQL XML input.
 *
 * This test verifies that the JSONWriter throws a runtime error
 * when attempting to write an invalid SPARQL XML format. The test
 * uses an invalid XML string to simulate erroneous input and expects
 * the writeToJson method to throw a std::runtime_error.
 */
TEST_F(JSONWriterTest, InvalidSparqlXml) {
    std::string invalid_sparql_xml = R"(<sparql><invalid></sparql>)";

    EXPECT_THROW(
        jsonWriter.writeToJson(invalid_sparql_xml, DataQueryAcceptType::SPARQL_XML, std::nullopt),
        std::runtime_error);
}