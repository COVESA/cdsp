#include <gtest/gtest.h>

#include "json_writer.h"
#include "model_config_fixture.h"
#include "random_utils.h"

class JSONWriterIntegrationTest : public ::testing::Test {
   protected:
    std::string data_point_1_;
    std::string data_point_2_;
    std::string data_point_3_;
    int value_dp_1_row_1_;
    float value_dp_1_row_2_;
    std::string value_dp_2_row_1_;
    std::string value_dp_2_row_2_;
    bool value_dp_3_row_1_;
    double value_dp_3_row_2_;
    nlohmann::json expected_json_;

    void SetUp() override {
        const std::string some_string_1 = RandomUtils::generateRandomString(4);
        const std::string some_string_2 = RandomUtils::generateRandomString(4);
        const std::string some_string_3 = RandomUtils::generateRandomString(4);
        const std::string some_string_4 = RandomUtils::generateRandomString(4);
        data_point_1_ = some_string_1 + "_" + some_string_2 + "_" + some_string_3;
        data_point_2_ = some_string_1 + "_" + some_string_4;
        data_point_3_ = some_string_3;
        value_dp_1_row_1_ = RandomUtils::generateRandomInt(0, 100);
        value_dp_1_row_2_ = RandomUtils::generateRandomFloat(0, 10);
        value_dp_2_row_1_ = RandomUtils::generateRandomString(10);
        value_dp_2_row_2_ = "";
        value_dp_3_row_1_ = RandomUtils::generateRandomInt(0, 1);
        value_dp_3_row_2_ = RandomUtils::generateRandomDouble(0, 10);

        expected_json_ = nlohmann::json::array(
            {{{some_string_1 + "." + some_string_2 + "." + some_string_3, value_dp_1_row_1_},
              {some_string_1 + "." + some_string_4, value_dp_2_row_1_},
              {some_string_3, value_dp_3_row_1_}},
             {{some_string_1 + "." + some_string_2 + "." + some_string_3, value_dp_1_row_2_},
              {some_string_1 + "." + some_string_4, value_dp_2_row_2_},
              {some_string_3, value_dp_3_row_2_}}});
    }

    // Helper function to validate the JSON output
    void validateJsonOutput(const nlohmann::json& actual_json) {
        ASSERT_TRUE(actual_json.contains("created_at"));
        ASSERT_TRUE(actual_json["created_at"].is_string());

        for (size_t i = 0; i < expected_json_.size(); i++) {
            for (auto& [key, expected_value] : expected_json_[i].items()) {
                auto actual_value = actual_json["results"][i][key];

                if (expected_value.is_number_float() && actual_value.is_number_float()) {
                    ASSERT_NEAR(actual_value.get<double>(), expected_value.get<double>(), 1e-6)
                        << "Mismatch in key: " << key;
                } else {
                    ASSERT_EQ(actual_value, expected_value) << "Mismatch in key: " << key;
                }
            }
        }
    }
};

/**
 * @brief Test case for converting SPARQL CSV results to JSON format.
 *
 * This test constructs a CSV string from predefined data points and values,
 * then uses the JSONWriter to convert the CSV string into a JSON object.
 * The resulting JSON is validated to ensure correct conversion.
 */
TEST_F(JSONWriterIntegrationTest, ConvertSparqlCsvToJson) {
    const std::string csv_result =
        data_point_1_ + "," + data_point_2_ + "," + data_point_3_ + "\r\n" +
        std::to_string(value_dp_1_row_1_) + "," + value_dp_2_row_1_ + "," +
        (value_dp_3_row_1_ ? "true" : "false") + "\r\n" + std::to_string(value_dp_1_row_2_) + "," +
        value_dp_2_row_2_ + "," + std::to_string(value_dp_3_row_2_);

    nlohmann::json actual_json =
        JSONWriter::writeToJson(csv_result, DataQueryAcceptType::TEXT_CSV, TEST_OUTPUT_DIR);

    validateJsonOutput(actual_json);
};

/**
 * @brief Test case for converting SPARQL TSV results to JSON format.
 *
 * This test constructs a TSV string from predefined data points and values,
 * then uses the JSONWriter to convert the TSV string into a JSON object.
 * The resulting JSON is validated to ensure correct conversion.
 */
TEST_F(JSONWriterIntegrationTest, ConvertSparqlTsvToJson) {
    const std::string tsv_result =
        data_point_1_ + "\t" + data_point_2_ + "\t" + data_point_3_ + "\r\n" +
        std::to_string(value_dp_1_row_1_) + "\t" + value_dp_2_row_1_ + "\t" +
        (value_dp_3_row_1_ ? "true" : "false") + "\r\n" + std::to_string(value_dp_1_row_2_) + "\t" +
        value_dp_2_row_2_ + "\t" + std::to_string(value_dp_3_row_2_);

    nlohmann::json actual_json =
        JSONWriter::writeToJson(tsv_result, DataQueryAcceptType::TEXT_TSV, TEST_OUTPUT_DIR);

    validateJsonOutput(actual_json);
};

/**
 * @brief Test case for converting SPARQL JSON results to JSON format.
 *
 * This test constructs a JSON string from predefined data points and values,
 * then uses the JSONWriter to convert the JSON string into a JSON object.
 * The resulting JSON is validated to ensure correct conversion.
 */
TEST_F(JSONWriterIntegrationTest, ConvertSparqlJsonToJson) {
    std::string sparql_json_result =
        R"({
        "head": {
            "vars": [" )" +
        data_point_1_ + R"(", ")" + data_point_2_ + R"(", ")" + data_point_3_ + R"("]
        },
        "results": {
            "bindings": [
                {
                    ")" +
        data_point_1_ + R"(": { "type": "literal", "value": ")" +
        std::to_string(value_dp_1_row_1_) + R"(" },
                    ")" +
        data_point_2_ + R"(": { "type": "uri", "value": ")" + value_dp_2_row_1_ + R"(" },
                    ")" +
        data_point_3_ + R"(": { "type": "literal", "value": ")" +
        (value_dp_3_row_1_ ? "true" : "false") + R"(" }
                },
                {
                    ")" +
        data_point_1_ + R"(": { "type": "bnode", "value": ")" + std::to_string(value_dp_1_row_2_) +
        R"(" },
                    ")" +
        data_point_2_ + R"(": { "type": "literal", "value": ")" + value_dp_2_row_2_ + R"(" },
                    ")" +
        data_point_3_ + R"(": { "type": "literal", "value": ")" +
        std::to_string(value_dp_3_row_2_) + R"(" }
                }
            ]
        }
    })";

    nlohmann::json actual_json = JSONWriter::writeToJson(
        sparql_json_result, DataQueryAcceptType::SPARQL_JSON, TEST_OUTPUT_DIR);

    validateJsonOutput(actual_json);
}

/**
 * @brief Test case for converting SPARQL XML results to JSON format.
 *
 * This test constructs an XML string from predefined data points and values,
 * then uses the JSONWriter to convert the XML string into a JSON object.
 * The resulting JSON is validated to ensure correct conversion.
 */
TEST_F(JSONWriterIntegrationTest, ConvertSparqXmlToJson) {
    const std::string xml_response_ =
        R"(<?xml version="1.0"?>
            <sparql xmlns="http://www.w3.org/2005/sparql-results#">
                <head>
                    <variable name=")" +
        data_point_1_ +
        R"("/>
                    <variable name=")" +
        data_point_2_ +
        R"("/>
                    <variable name=")" +
        data_point_3_ +
        R"("/>
                </head>
                <results>
                    <result>
                        <binding name=")" +
        data_point_1_ +
        R"(">
                            <uri>)" +
        std::to_string(value_dp_1_row_1_) +
        R"(</uri>
                        </binding>
                        <binding name=")" +
        data_point_2_ +
        R"(">
                            <literal>)" +
        value_dp_2_row_1_ +
        R"(</literal>
                        </binding>
                        <binding name=")" +
        data_point_3_ +
        R"(">
                            <uri>)" +
        (value_dp_3_row_1_ ? "true" : "false") +
        R"(</uri>
                        </binding>
                    </result>
                    <result>
                        <binding name=")" +
        data_point_1_ +
        R"(">
                            <bnode>)" +
        std::to_string(value_dp_1_row_2_) +
        R"(</bnode>
                        </binding>
                        <binding name=")" +
        data_point_2_ +
        R"(">
                            <uri>)" +
        value_dp_2_row_2_ +
        R"(</uri>
                        </binding>
                        <binding name=")" +
        data_point_3_ +
        R"(">
                            <uri>)" +
        std::to_string(value_dp_3_row_2_) +
        R"(</uri>
                        </binding>
                    </result>
                </results>
            </sparql>)";

    nlohmann::json actual_json =
        JSONWriter::writeToJson(xml_response_, DataQueryAcceptType::SPARQL_XML, TEST_OUTPUT_DIR);

    validateJsonOutput(actual_json);
}