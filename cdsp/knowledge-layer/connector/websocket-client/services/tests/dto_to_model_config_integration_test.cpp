#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "data_points_utils.h"
#include "dto_to_bo.h"
#include "globals.h"
#include "mock_i_file_handler.h"
#include "model_config_converter.h"
#include "random_utils.h"
#include "vin_utils.h"

class DtoToModelConfigIntegrationTest : public ::testing::Test {
   protected:
    std::shared_ptr<MockIFileHandler> mock_i_file_handler_;
    std::unique_ptr<DtoToBo> dto_to_bo_;
    const std::vector<SchemaType> SCHEMAS_IN_TEST = {SchemaType::VEHICLE};
    const std::vector<std::string> ENGINES_IN_TEST = {"rdfox"};
    const std::vector<std::pair<std::string, std::string>> SYNTAX_TYPES_AND_EXTENSION_IN_TEST = {
        {"turtle", "ttl"}, {"trig", "trig"}, {"ntriples", "nt"}, {"nquads", "nq"}};

    const std::vector<std::pair<std::string, std::string>>
        QUERY_LANGUAGE_TYPES_AND_EXTENSION_IN_TEST = {{"sparql", "rq"}};

    const std::vector<std::pair<std::string, std::string>> REASONER_RULES_AND_EXTENSIONS_IN_TEST = {
        {"datalog", "dlog"}};

    const std::string MODEL_CONFIG_PATH =
        getProjectRoot() + "/symbolic-reasoner/examples/use-case/model/";

    void SetUp() override {
        // ** Initialize Main Services **
        setenv("VEHICLE_OBJECT_ID", VinUtils::getRandomVinString().c_str(), 1);

        // Initialize the mock file handler
        mock_i_file_handler_ = std::make_shared<MockIFileHandler>();

        // Initialize the DtoToBo with the mock file handler
        dto_to_bo_ = std::make_unique<DtoToBo>(mock_i_file_handler_);
    }
};

/**
 * @brief Selects a random type and file extension pair.
 *
 * This function takes a list of type and file extension pairs and returns
 * one randomly selected pair from the list. The selection is made using a
 * random index generated within the bounds of the list size.
 *
 * @param types_list_to_use A vector containing pairs of strings, where each pair
 *        consists of a type and its corresponding file extension.
 *
 * @return std::pair<std::string, std::string> A randomly selected pair from the input list,
 *         containing a type and its corresponding file extension.
 */
std::pair<std::string, std::string> getRandomTypeAndFileExtension(
    std::vector<std::pair<std::string, std::string>> types_list_to_use) {
    int random_index = RandomUtils::generateRandomInt(0, types_list_to_use.size() - 1);
    return types_list_to_use[random_index];
}

/**
 * Generates a random list of file types and file paths.
 *
 * This function creates a vector of pairs, where each pair consists of a file type
 * and a randomly generated file path with an appropriate extension. The number of
 * files generated is random, ranging from 1 to the specified maximum number of files.
 *
 * @param type_list_to_use A vector of pairs, where each pair contains a file type
 *                         and its corresponding file extension.
 * @param max_files The maximum number of files to generate. Defaults to 5.
 * @return A vector of pairs, where each pair contains a file type and a randomly
 *         generated file path with the corresponding file extension.
 */
std::vector<std::pair<std::string, std::string>> generateRandomListOfTypeAndFilePaths(
    std::vector<std::pair<std::string, std::string>> type_list_to_use, int max_files = 5) {
    int num_files = RandomUtils::generateRandomInt(1, max_files);
    std::vector<std::pair<std::string, std::string>> file_paths;
    for (int i = 0; i < num_files; i++) {
        auto rdf_extension = getRandomTypeAndFileExtension(type_list_to_use);
        file_paths.push_back({rdf_extension.first,
                              RandomUtils::generateRandomString() + "." + rdf_extension.second});
    }
    return file_paths;
}

ModelConfigDTO createValidDto() {
    ModelConfigDTO dto;
    dto.inputs = {{"Vehicle_data", "path/some_input.txt"}};
    dto.ontologies = {"path/some_ontology.ttl"};
    dto.output = "path/output/";
    dto.rules = {"path/rules.dlog"};
    dto.shacl_shapes = {"path/shapes.ttl"};
    dto.queries.triple_assembler_helper = {
        {"Vehicle", {"path/data_property.rq", "path/object_property.rq"}},
        {"default", {"path/data_property.rq", "path/object_property.rq"}}};
    dto.queries.reasoning_output_queries_path = "path/output_queries/";
    dto.reasoner_settings.inference_engine = "rdfox";
    dto.reasoner_settings.output_format = "turtle";
    dto.reasoner_settings.supported_schema_collections = {"Vehicle"};
    return dto;
}

/**
 * @brief Test case for converting a ModelConfigDTO to a ModelConfig business object.
 *
 * This test verifies that the conversion from a ModelConfigDTO to a ModelConfig
 * business object is performed correctly. It checks that all fields are accurately
 * transferred from the DTO to the BO.
 */
TEST_F(DtoToModelConfigIntegrationTest, ConvertModelConfigDtoToBo) {
    // Arrange
    // Generate model config inputs
    std::map<std::string, std::string> dto_random_inputs;
    std::map<SchemaType, std::vector<std::string>> expected_bo_inputs;
    for (const auto& schema : SCHEMAS_IN_TEST) {
        std::string input_path = RandomUtils::generateRandomString() + ".txt";
        std::string return_from_file;
        for (int i = 0; i < RandomUtils::generateRandomInt(1, 5); i++) {
            expected_bo_inputs[schema].push_back(DataPointsUtils::generateRandomKey());
            return_from_file += expected_bo_inputs[schema].back() + "\n";
        }

        dto_random_inputs[schemaTypeToString(schema, true) + ModelConfigConverter::INPUT_SUFFIX] =
            input_path;

        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + input_path)))
            .WillOnce(testing::Return(return_from_file));
    }

    // Generate model config ontologies
    std::vector<std::string> dto_ontologies;
    std::vector<std::pair<ReasonerSyntaxType, std::string>> expected_bo_ontologies;
    for (const auto& ontology_type_and_path :
         generateRandomListOfTypeAndFilePaths(SYNTAX_TYPES_AND_EXTENSION_IN_TEST)) {
        dto_ontologies.push_back(ontology_type_and_path.second);
        expected_bo_ontologies.push_back(
            std::make_pair(reasonerOutputFormatToReasonerSyntaxType(ontology_type_and_path.first),
                           RandomUtils::generateRandomString()));

        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + ontology_type_and_path.second)))
            .WillOnce(testing::Return(expected_bo_ontologies.back().second));
    }

    // Generate model config output
    std::string dto_output = RandomUtils::generateRandomString();
    std::string expected_bo_output = MODEL_CONFIG_PATH + dto_output;

    // Generate model config queries for schema types
    QueriesDTO dto_queries;
    std::map<SchemaType, TripleAssemblerHelper::QueryPair>
        expected_bo_triple_assembler_helper_queries;

    for (const auto& schema : SCHEMAS_IN_TEST) {
        auto random_query_extension =
            getRandomTypeAndFileExtension(QUERY_LANGUAGE_TYPES_AND_EXTENSION_IN_TEST);

        expected_bo_triple_assembler_helper_queries[schema] = TripleAssemblerHelper::QueryPair{
            std::make_pair(fileExtensionToQueryLanguageType("." + random_query_extension.second),
                           RandomUtils::generateRandomString()),
            std::make_pair(fileExtensionToQueryLanguageType("." + random_query_extension.second),
                           RandomUtils::generateRandomString())};

        std::string data_property_file_path =
            RandomUtils::generateRandomString() + "/data_property." + random_query_extension.second;
        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + data_property_file_path)))
            .WillOnce(testing::Return(
                expected_bo_triple_assembler_helper_queries[schema].data_property.second));

        std::string object_property_file_path = RandomUtils::generateRandomString() +
                                                "/object_property." + random_query_extension.second;
        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + object_property_file_path)))
            .WillOnce(testing::Return(
                expected_bo_triple_assembler_helper_queries[schema].object_property.second));

        std::vector<std::string> random_query_triple_helper_file_paths = {
            data_property_file_path, object_property_file_path};

        dto_queries.triple_assembler_helper[schemaTypeToString(schema, true)] =
            random_query_triple_helper_file_paths;
    }

    // Generate model config queries for default schema type
    auto generateDefaultFiles = RandomUtils::generateRandomBool();

    if (generateDefaultFiles) {
        auto random_query_extension =
            getRandomTypeAndFileExtension(QUERY_LANGUAGE_TYPES_AND_EXTENSION_IN_TEST);

        expected_bo_triple_assembler_helper_queries[SchemaType::DEFAULT] =
            TripleAssemblerHelper::QueryPair{
                std::make_pair(
                    fileExtensionToQueryLanguageType("." + random_query_extension.second),
                    RandomUtils::generateRandomString()),
                std::make_pair(
                    fileExtensionToQueryLanguageType("." + random_query_extension.second),
                    RandomUtils::generateRandomString())};

        std::string data_property_file_path =
            RandomUtils::generateRandomString() + "/data_property." + random_query_extension.second;

        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + data_property_file_path)))
            .WillOnce(
                testing::Return(expected_bo_triple_assembler_helper_queries[SchemaType::DEFAULT]
                                    .data_property.second));

        std::string object_property_file_path = RandomUtils::generateRandomString() +
                                                "/object_property." + random_query_extension.second;

        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + object_property_file_path)))
            .WillOnce(
                testing::Return(expected_bo_triple_assembler_helper_queries[SchemaType::DEFAULT]
                                    .object_property.second));

        std::vector<std::string> random_query_triple_helper_file_paths = {
            data_property_file_path, object_property_file_path};

        dto_queries.triple_assembler_helper["default"] = random_query_triple_helper_file_paths;
    } else {
        dto_queries.triple_assembler_helper["default"] = {};
    }

    // Generate reasoning output queries
    dto_queries.reasoning_output_queries_path = RandomUtils::generateRandomString();
    std::vector<std::string> dto_reasoning_output_queries_paths;
    std::vector<std::pair<QueryLanguageType, std::string>> expected_bo_reasoning_output_queries;

    for (int i = 0; i < RandomUtils::generateRandomInt(1, 5); i++) {
        auto random_query_extension =
            getRandomTypeAndFileExtension(QUERY_LANGUAGE_TYPES_AND_EXTENSION_IN_TEST);

        expected_bo_reasoning_output_queries.push_back(
            std::make_pair(fileExtensionToQueryLanguageType("." + random_query_extension.second),
                           RandomUtils::generateRandomString()));

        dto_reasoning_output_queries_paths.push_back(RandomUtils::generateRandomString() + "." +
                                                     random_query_extension.second);

        std::string expected_bo_reasoning_output_query =
            MODEL_CONFIG_PATH + dto_queries.reasoning_output_queries_path + "/" +
            dto_reasoning_output_queries_paths.back();

        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(expected_bo_reasoning_output_query)))
            .WillOnce(testing::Return(expected_bo_reasoning_output_queries.back().second));
    }

    EXPECT_CALL(*mock_i_file_handler_,
                readDirectory(::testing::StrEq(MODEL_CONFIG_PATH +
                                               dto_queries.reasoning_output_queries_path)))
        .WillOnce(testing::Return(dto_reasoning_output_queries_paths));

    // Generate model config rules
    std::vector<std::string> dto_rules;
    std::vector<std::pair<RuleLanguageType, std::string>> expected_bo_rules;
    for (int i = 0; i < RandomUtils::generateRandomInt(1, 5); i++) {
        auto random_role_extension =
            getRandomTypeAndFileExtension(REASONER_RULES_AND_EXTENSIONS_IN_TEST);

        expected_bo_rules.push_back(
            std::make_pair(fileExtensionToRuleLanguageType("." + random_role_extension.second),
                           RandomUtils::generateRandomString()));

        dto_rules.push_back(RandomUtils::generateRandomString() + "." +
                            random_role_extension.second);

        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + dto_rules.back())))
            .WillOnce(testing::Return(expected_bo_rules.back().second));
    }

    // Generate model config validation shapes
    std::vector<std::string> dto_shacl_shapes;
    std::vector<std::pair<ReasonerSyntaxType, std::string>> expected_bo_validation_shapes;
    for (const auto& shacl_file_path :
         generateRandomListOfTypeAndFilePaths(SYNTAX_TYPES_AND_EXTENSION_IN_TEST)) {
        dto_shacl_shapes.push_back(shacl_file_path.second);
        expected_bo_validation_shapes.push_back(
            std::make_pair(reasonerOutputFormatToReasonerSyntaxType(shacl_file_path.first),
                           RandomUtils::generateRandomString()));

        EXPECT_CALL(*mock_i_file_handler_,
                    readFile(::testing::StrEq(MODEL_CONFIG_PATH + shacl_file_path.second)))
            .WillOnce(testing::Return(expected_bo_validation_shapes.back().second));
    }

    // Generate model config reasoner settings
    ReasonerSettingsDTO dto_reasoner_settings;
    dto_reasoner_settings.inference_engine =
        ENGINES_IN_TEST[RandomUtils::generateRandomInt(0, ENGINES_IN_TEST.size() - 1)];

    InferenceEngineType expected_bo_reasoner_inference_engine =
        stringToInferenceEngineType(dto_reasoner_settings.inference_engine);

    dto_reasoner_settings.output_format =
        getRandomTypeAndFileExtension(SYNTAX_TYPES_AND_EXTENSION_IN_TEST).first;

    ReasonerSyntaxType expected_bo_reasoner_output_format =
        reasonerOutputFormatToReasonerSyntaxType(dto_reasoner_settings.output_format);

    std::vector<SchemaType> expected_bo_reasoner_supported_schema_collections;

    for (SchemaType schema : SCHEMAS_IN_TEST) {
        dto_reasoner_settings.supported_schema_collections.push_back(
            schemaTypeToString(schema, true));
        expected_bo_reasoner_supported_schema_collections.push_back(schema);
    }

    // Create a ModelConfigDTO
    ModelConfigDTO dto;
    dto.inputs = dto_random_inputs;
    dto.ontologies = dto_ontologies;
    dto.output = dto_output;
    dto.queries = dto_queries;
    dto.rules = dto_rules;
    dto.shacl_shapes = dto_shacl_shapes;
    dto.reasoner_settings = dto_reasoner_settings;
    std::cout << "ModelConfigDTO to parse: \n" << dto << std::endl;

    // Act
    auto bo = dto_to_bo_->convert(dto);
    std::cout << "ModelConfig parsed: \n" << bo << std::endl;

    // Assert
    EXPECT_EQ(bo.getInputs(), expected_bo_inputs);
    EXPECT_EQ(bo.getOntologies(), expected_bo_ontologies);
    EXPECT_EQ(bo.getOutput(), expected_bo_output);
    EXPECT_EQ(bo.getReasonerRules(), expected_bo_rules);
    EXPECT_EQ(bo.getValidationShapes(), expected_bo_validation_shapes);

    TripleAssemblerHelper bo_queries_config = bo.getQueriesTripleAssemblerHelper();
    for (const auto& [expected_bo_schema, expected_bo_query_pair] :
         expected_bo_triple_assembler_helper_queries) {
        EXPECT_EQ(bo_queries_config.getQueries()[expected_bo_schema].data_property,
                  expected_bo_query_pair.data_property);

        EXPECT_EQ(bo_queries_config.getQueries()[expected_bo_schema].object_property,
                  expected_bo_query_pair.object_property);
    }
    EXPECT_EQ(bo.getReasoningOutputQueries(), expected_bo_reasoning_output_queries);

    ReasonerSettings bo_reasoner_settings = bo.getReasonerSettings();
    EXPECT_EQ(bo_reasoner_settings.getInferenceEngine(), expected_bo_reasoner_inference_engine);
    EXPECT_EQ(bo_reasoner_settings.getOutputFormat(), expected_bo_reasoner_output_format);

    for (const SchemaType& schema : expected_bo_reasoner_supported_schema_collections) {
        EXPECT_THAT(bo_reasoner_settings.getSupportedSchemaCollections(),
                    testing::Contains(schema));
    }
}

/**
 * @brief Test case for converting a ModelConfigDTO with missing required fields.
 *
 * This test verifies that attempting to convert a ModelConfigDTO to a business object (BO)
 * throws an invalid_argument exception when any of the required fields are missing.
 */
TEST_F(DtoToModelConfigIntegrationTest,
       ConvertModelConfigDtoThrowsExceptionWhenRequiredFieldsAreMissing) {
    // Define the required fields
    std::vector<std::string> required_fields = {"inputs",
                                                "output",
                                                "shacl_shapes",
                                                "rules",
                                                "inference_engine",
                                                "output_format",
                                                "supported_schema_collections",
                                                "triple_assembler_helper",
                                                "reasoning_output_queries"};

    // Arrange

    // For this test, the file handler is irrelevant and return a dummy string
    EXPECT_CALL(*mock_i_file_handler_, readFile(::testing::_))
        .WillRepeatedly(testing::Return("some_data"));

    EXPECT_CALL(*mock_i_file_handler_, readDirectory(::testing::_))
        .WillRepeatedly(testing::Return(std::vector<std::string>()));

    for (const auto& field : required_fields) {
        // Define the complete ModelConfigDTO
        ModelConfigDTO dto = createValidDto();
        std::string error_message;

        // Set the field to an empty value
        if (field == "inputs") {
            dto.inputs.clear();
            error_message = "Inputs map cannot be empty";
        } else if (field == "output") {
            dto.output = std::string();
            error_message = "Output path cannot be empty";
        } else if (field == "shacl_shapes") {
            dto.shacl_shapes.clear();
            error_message = "Validation shapes cannot be empty";
        } else if (field == "rules") {
            dto.rules.clear();
            error_message = "Reasoner rules cannot be empty";
        } else if (field == "inference_engine") {
            dto.reasoner_settings.inference_engine = std::string();
            error_message = "Reasoner settings fields cannot be empty";
        } else if (field == "output_format") {
            dto.reasoner_settings.output_format = std::string();
            error_message = "Reasoner settings fields cannot be empty";
        } else if (field == "supported_schema_collections") {
            dto.reasoner_settings.supported_schema_collections.clear();
            error_message = "Supported schema collections cannot be empty";
        } else if (field == "triple_assembler_helper") {
            dto.queries.triple_assembler_helper.clear();
            error_message =
                "Queries for the triple assembler helper cannot be empty. At least one query must "
                "be provided for each schema or default";
        } else if (field == "reasoning_output_queries") {
            dto.queries.reasoning_output_queries_path = std::string();
            error_message = "Reasoning output queries cannot be empty";
        }

        std::cout << "\nTesting with missing field: " << field << "\n";
        std::cout << dto << std::endl;

        // Act & Assert: Ensure exception is thrown when a required field is missing
        ASSERT_THROW(
            {
                try {
                    dto_to_bo_->convert(dto);
                } catch (const std::invalid_argument& e) {
                    EXPECT_THAT(e.what(), testing::HasSubstr(error_message));
                    throw;
                }
            },
            std::invalid_argument);
    }
}

/**
 * @brief Tests the conversion of ModelConfigDTO with incomplete queries.
 *
 * This test verifies that the conversion process of a ModelConfigDTO handles
 * incomplete queries correctly. It checks that an exception is thrown when
 * required fields are missing for certain schema types, while ensuring no
 * exception is thrown for the "default" schema type.
 */
TEST_F(DtoToModelConfigIntegrationTest, ConvertModelConfigDtoHandlesIncompleteQueriesCorrectly) {
    // Define the required fields
    std::vector<std::string> required_fields = {"data_property", "object_property"};
    std::vector<std::string> schema_types = {"Vehicle", "default"};
    std::string error_message;

    // For this test, the file handler is irrelevant and return a dummy string
    EXPECT_CALL(*mock_i_file_handler_, readFile(::testing::_))
        .WillRepeatedly(testing::Return("some_data"));

    EXPECT_CALL(*mock_i_file_handler_, readDirectory(::testing::_))
        .WillRepeatedly(testing::Return(std::vector<std::string>({"some_file.rq"})));

    for (const auto& schema : schema_types) {
        for (const auto& field : required_fields) {
            // Define the complete ModelConfigDTO
            ModelConfigDTO dto = createValidDto();

            // Set the field to an empty value
            auto& files = dto.queries.triple_assembler_helper[schema];
            if (field == "data_property") {
                files.erase(files.begin());
            } else if (field == "object_property") {
                files.erase(files.begin() + 1);
            }

            error_message = "Failed to read data and object properties for `" + schema +
                            "` collection from the model config";

            std::cout << "\nTesting with failing " + field + " queries for schema: " << schema
                      << "\n";
            std::cout << dto << std::endl;

            // Act & Assert: Ensure exception is thrown when a required field is missing
            if (schema == "default") {
                ASSERT_NO_THROW({
                    auto bo = dto_to_bo_->convert(dto);
                    std::cout << "ModelConfig parsed: \n" << bo << std::endl;
                    auto query = bo.getQueriesTripleAssemblerHelper().getQueries();
                    for (const auto& [schema, query_pair] : query) {
                        std::cout << "Schema: " << schemaTypeToString(schema) << "\n";
                    }
                    EXPECT_NE(query.find(SchemaType::VEHICLE), query.end());
                    EXPECT_EQ(query.find(SchemaType::DEFAULT), query.end());
                });

            } else {
                ASSERT_THROW(
                    {
                        try {
                            dto_to_bo_->convert(dto);
                        } catch (const std::invalid_argument& e) {
                            EXPECT_THAT(e.what(), testing::HasSubstr(error_message));
                            throw;
                        }
                    },
                    std::invalid_argument);
            }
        }
    }
}
