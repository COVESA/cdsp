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

// Helper function to generate a random map of strings
std::map<std::string, std::string> generateRandomMapOfStrings(int min_size = 1) {
    int num_inputs = RandomUtils::generateRandomInt(min_size, 5);
    std::map<std::string, std::string> inputs;

    for (int i = 0; i < num_inputs; ++i) {
        int string_length = RandomUtils::generateRandomInt(5, 10);
        inputs[RandomUtils::generateRandomString(string_length)] =
            "inputs/" + RandomUtils::generateRandomString(string_length);
    }

    return inputs;
}

// Helper function to generate a random vector of strings
std::vector<std::string> generateRandomVector(int min_size = 0, const std::string& prefix = "") {
    int num_inputs = RandomUtils::generateRandomInt(min_size, 5);

    std::vector<std::string> inputs;

    for (int i = 0; i < num_inputs; ++i) {
        int string_length = RandomUtils::generateRandomInt(5, 10);
        inputs.push_back(prefix + RandomUtils::generateRandomString(string_length));
    }

    return inputs;
}

// Helper function to generate a random map of vectors of strings
std::map<std::string, std::vector<std::string>> generateRandomMapOfStringsVector(
    int min_size_map = 1, int min_size_vec = 1, const std::string& prefix = "") {
    int num_inputs_map = RandomUtils::generateRandomInt(min_size_map, 5);
    int num_inputs_vector = RandomUtils::generateRandomInt(min_size_vec, 5);
    std::map<std::string, std::vector<std::string>> inputs;

    for (int i = 0; i < num_inputs_map; ++i) {
        std::vector<std::string> random_queries;
        int string_length_key = RandomUtils::generateRandomInt(5, 10);

        for (int j = 0; j < num_inputs_vector; ++j) {
            int string_length = RandomUtils::generateRandomInt(5, 10);

            random_queries.push_back(prefix + RandomUtils::generateRandomString(string_length));
        }
        inputs[RandomUtils::generateRandomString(string_length_key)] = random_queries;
    }

    return inputs;
}

nlohmann::json generateValidModelConfigJson() {
    return {{"inputs", {{"vehicle_data", "inputs/vehicle_data_required.txt"}}},
            {"ontologies", {"ontologies/example_ontology.ttl"}},
            {"output", "output/"},
            {"queries",
             {{"triple_assembler_helper",
               {{"vehicle",
                 {"queries/triple_assembler_helper/vehicle/data_property.rq",
                  "queries/triple_assembler_helper/vehicle/object_property.rq"}},
                {"default", nlohmann::json::array()}}},
              {"output", "queries/output/"}}},
            {"rules", {"rules/insight_rules.ttl"}},
            {"shacl", {"shacl/vehicle_shacl.ttl", "shacl/observation_shacl.ttl"}},
            {"reasoner_settings",
             {{"inference_engine", "RDFox"},
              {"output_format", "turtle"},
              {"supported_schema_collections", {"vehicle"}},
              {"is_ai_reasoner_inference_results", true}}}};
}

/**
 * @brief Test case for parsing a valid ModelConfig JSON with random values.
 *
 * This test verifies the parsing of a valid ModelConfig JSON object into a ModelConfigDTO.
 * It ensures that the generated random values match the parsed DTO values.
 */
TEST_F(ModelConfigDtoServiceUnitTest, ParseModelConfigWithRandomValues) {
    // Arrange: Generate random data while preserving the correct structure
    auto random_inputs = generateRandomMapOfStrings();
    auto random_ontologies = generateRandomVector(1, "ontologies/");
    auto random_output = "output/" + RandomUtils::generateRandomString(10) + "/";

    auto random_reasoning_output_queries_path =
        "queries/" + RandomUtils::generateRandomString(10) + "/";
    auto random_triple_assembler_helper = generateRandomMapOfStringsVector(1, 2, "queries/");

    auto random_rules = generateRandomVector(1, "rules/");
    auto random_shacl = generateRandomVector(2, "shacl/");

    auto random_inference_engine = RandomUtils::generateRandomString(10);
    auto random_output_format = RandomUtils::generateRandomString(10);
    auto random_supported_schema_collections = generateRandomVector(1, "schema_");
    bool random_is_ai_reasoner_inference_results = RandomUtils::generateRandomBool();

    // Build the expected JSON structure with random values
    nlohmann::json json_message = {
        {"inputs", random_inputs},
        {"ontologies", random_ontologies},
        {"output", random_output},
        {"queries",
         {{"triple_assembler_helper", random_triple_assembler_helper},
          {"output", random_reasoning_output_queries_path}}},
        {"rules", random_rules},
        {"shacl", random_shacl},
        {"reasoner_settings",
         {{"inference_engine", random_inference_engine},
          {"output_format", random_output_format},
          {"supported_schema_collections", random_supported_schema_collections},
          {"is_ai_reasoner_inference_results", random_is_ai_reasoner_inference_results}}}};

    std::cout << "Incoming random message: \n" << json_message.dump(4) << std::endl;

    // Act: Parse the random JSON into a ModelConfigDTO
    ModelConfigDTO dto = dto_service_.parseModelConfigJsonToDto(json_message);
    std::cout << "Parsed ModelConfigDTO with random values: \n" << dto << std::endl;

    // Assert: Validate the parsed data against the randomly generated values
    ASSERT_EQ(dto.inputs, random_inputs);
    ASSERT_EQ(dto.ontologies, random_ontologies);
    ASSERT_EQ(dto.output, random_output);

    ASSERT_EQ(dto.queries.triple_assembler_helper, random_triple_assembler_helper);
    ASSERT_EQ(dto.queries.reasoning_output_queries_path, random_reasoning_output_queries_path);

    ASSERT_EQ(dto.rules, random_rules);
    ASSERT_EQ(dto.shacl_shapes, random_shacl);

    ASSERT_EQ(dto.reasoner_settings.inference_engine, random_inference_engine);
    ASSERT_EQ(dto.reasoner_settings.output_format, random_output_format);
    ASSERT_EQ(dto.reasoner_settings.supported_schema_collections,
              random_supported_schema_collections);

    ASSERT_EQ(dto.reasoner_settings.is_ai_reasoner_inference_results,
              random_is_ai_reasoner_inference_results);
}

/**
 * @brief Unit test for the DtoService class to verify exception handling.
 *
 * This test case checks that the `parseModelConfigJsonToDto` method of the `dto_service_` object
 * throws an `std::invalid_argument` exception when any of the required fields are missing
 * from the JSON message. The required fields are "inputs", "ontologies", "output", "queries",
 * "rules", "shacl", and "reasoner_settings".
 *
 * The test iterates over each required field, removes it from a valid JSON message,
 * and asserts that an exception is thrown when the modified message is parsed.
 */
TEST_F(ModelConfigDtoServiceUnitTest,
       ParseModelConfigDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
    // Define the complete JSON message
    nlohmann::json json_message = generateValidModelConfigJson();

    // List of required fields in the JSON message
    std::vector<std::string> required_fields = {
        "inputs", "ontologies", "output", "queries", "rules", "shacl", "reasoner_settings"};

    // Iterate over each required field
    for (const auto& field : required_fields) {
        nlohmann::json test_message = json_message;
        test_message.erase(field);  // Remove the required field

        // Output the current test case details
        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << test_message.dump(4) << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(dto_service_.parseModelConfigJsonToDto(test_message), std::invalid_argument);
    }
}
