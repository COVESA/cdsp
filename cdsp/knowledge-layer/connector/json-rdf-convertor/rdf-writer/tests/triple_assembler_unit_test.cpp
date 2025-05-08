#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "data_message.h"
#include "data_types.h"
#include "mock_i_file_handler.h"
#include "mock_model_config.h"
#include "mock_reasoner_adapter.h"
#include "mock_reasoner_service.h"
#include "mock_triple_writer.h"
#include "model_config.h"
#include "triple_assembler.h"
#include "vin_utils.h"

class TripleAssemblerUnitTest : public ::testing::Test {
   protected:
    const std::string VIN = VinUtils::getRandomVinString();

    std::shared_ptr<TripleAssembler> triple_assembler_;

    // Set up mock ReasonerAdapter
    std::shared_ptr<MockReasonerAdapter> mock_adapter_;
    std::shared_ptr<MockModelConfig> mock_model_config_;
    std::shared_ptr<MockReasonerService> mock_reasoner_service_;
    MockIFileHandler mock_i_file_handler_;
    MockTripleWriter mock_triple_writer_;
    std::vector<Node> nodes_{};

    void SetUp() override {
        // ** Initialize Main Services **
        setenv("VEHICLE_OBJECT_ID", VinUtils::getRandomVinString().c_str(), 1);

        // Mock functions to be called and expectations
        mock_model_config_ = std::make_shared<MockModelConfig>();
        mock_adapter_ = std::make_shared<MockReasonerAdapter>();
        EXPECT_CALL(*mock_adapter_, initialize()).Times(1);
        mock_reasoner_service_ = std::make_shared<MockReasonerService>(mock_adapter_);

        // Initialize TripleAssembler
        triple_assembler_ = std::make_shared<TripleAssembler>(
            mock_model_config_, *mock_reasoner_service_, mock_i_file_handler_, mock_triple_writer_);
    }

    void TearDown() override {
        triple_assembler_.reset();
        mock_reasoner_service_.reset();
    }

    void setUpMessage() {
        const std::string data_point =
            "Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy";

        nodes_.emplace_back(data_point, "98.6", Metadata(), std::vector<std::string>{data_point});
    }

    /**
     * @brief Sets up initial expectations for mock objects and functions.
     *
     * This function configures the expected behavior and return values for various
     * mock objects and functions used in testing. It sets up expectations for
     * checking the data store, initiating triples, reading SHACL query files, and
     * querying data using SHACL queries.
     *
     * @param times_initiation The number of times the initiation functions are expected
     *        to be executed.
     * @param times_executing_object_related_functions The number of times the object-related
     *        functions are expected to be executed.
     * @param times_executing_data_related_functions The number of times the data-related
     *        functions are expected to be executed.
     * @param query_object_response The predefined response to return when querying
     *        data using the object-related SHACL query. Defaults to an empty string.
     * @param query_data_response The predefined response to return when querying
     *        data using the data-related SHACL query. Defaults to an empty string.
     */
    void initialSetupExpectations(const int& times_initiation,
                                  const int& times_executing_object_related_functions,
                                  const int& times_executing_data_related_functions,
                                  const std::string& query_object_response = "some_object_query",
                                  const std::string& query_data_response = "some_data_query") {
        // Mock data store has been setup
        EXPECT_CALL(*mock_reasoner_service_, checkDataStore())
            .Times(times_initiation)
            .WillRepeatedly(testing::Return(true));
        EXPECT_CALL(mock_triple_writer_, initiateTriple(VIN)).Times(times_initiation);

        // Define SHACL queries for object and data properties
        const std::string query_object = R"(prefix ex: <http://www.example.com#>
prefix otr: <http://www.other.com#>

SELECT some_object_query)";

        const std::string query_data = R"(prefix ex: <http://www.example.com#>
prefix so: <http://www.some.com#>

SELECT some_data_query)";

        // Mock querying data using the queries and returning predefined responses
        TripleAssemblerHelper::QueryPair query_pair;
        query_pair.object_property = std::make_pair(QueryLanguageType::SPARQL, query_object);
        query_pair.data_property = std::make_pair(QueryLanguageType::SPARQL, query_data);
        TripleAssemblerHelper triple_assembler_helper({{SchemaType::VEHICLE, query_pair}});

        EXPECT_CALL(*mock_model_config_, getQueriesTripleAssemblerHelper())
            .Times(times_executing_data_related_functions)
            .WillRepeatedly(testing::Return(triple_assembler_helper));

        EXPECT_CALL(*mock_reasoner_service_,
                    queryData(query_object, QueryLanguageType::SPARQL,
                              ::testing::Eq(DataQueryAcceptType::TEXT_TSV)))
            .Times(times_executing_object_related_functions)
            .WillRepeatedly(testing::Return(query_object_response));

        EXPECT_CALL(*mock_reasoner_service_,
                    queryData(query_data, QueryLanguageType::SPARQL,
                              ::testing::Eq(DataQueryAcceptType::TEXT_TSV)))
            .Times(times_executing_data_related_functions)
            .WillRepeatedly(testing::Return(query_data_response));
    }
};

MATCHER(IsOptionalWithValue, "Checks that std::optional contains a value") {
    return arg.has_value();
}

/**
 * @brief Unit test for successful initialization of the Triple Assembler.
 *
 * This test sets up the model base and mocks various components to simulate
 * the initialization process of the Triple Assembler. It verifies that the
 * initialization completes without throwing exceptions.
 */
TEST_F(TripleAssemblerUnitTest, InitializeSuccess) {
    // Mock the data store check to return true, indicating the data store is available
    EXPECT_CALL(*mock_reasoner_service_, checkDataStore()).WillOnce(testing::Return(true));

    // Mock reading SHACL shape files and returning predefined data
    EXPECT_CALL(*mock_model_config_, getValidationShapes())
        .Times(1)
        .WillOnce(testing::Return(std::vector<std::pair<ReasonerSyntaxType, std::string>>{
            {ReasonerSyntaxType::TURTLE, "data1"}, {ReasonerSyntaxType::NQUADS, "data2"}}));

    // Mock loading data into the reasoner service and returning success
    EXPECT_CALL(*mock_reasoner_service_,
                loadData(testing::StrEq("data1"), ReasonerSyntaxType::TURTLE))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(*mock_reasoner_service_,
                loadData(testing::StrEq("data2"), ReasonerSyntaxType::NQUADS))
        .WillOnce(testing::Return(true));

    // Assert that the initialization process does not throw any exceptions
    EXPECT_NO_THROW(triple_assembler_->initialize());
}

/**
 * @brief Unit test for transforming a message to triples successfully.
 *
 * This test sets up a message and model base, then mocks various components
 * to simulate the transformation of a message into triples. It verifies
 * that the transformation process completes with the expected behaviour without throwing
 * exceptions.
 */
TEST_F(TripleAssemblerUnitTest, TransformMessageToTripleSuccess) {
    // Set up the initial message and model base for the test
    setUpMessage();

    auto message_header = MessageHeader(VIN, SchemaType::VEHICLE);
    DataMessage message_feature(message_header, nodes_);

    // Define mock responses for the SHACL queries
    std::string query_object_response =
        R"(?class1 ?object_property        ?class2
 <http://www.example.com#object_class_1> <http://www.example.com#object_property_1> <http://www.example.com#object_class_2>)";

    std::string query_data_response =
        R"(?class1 ?data_property        ?datatype
 <http://www.example.com#object_class_1> <http://www.example.com#data_property_1> <http://www.some.com#datatype_1>)";

    // Set up the initial expectations for the test
    initialSetupExpectations(1, 3, 1, query_object_response, query_data_response);

    // Define tuples for RDF object and data elements
    const std::tuple<std::string, std::string, std::string> object_elements = std::make_tuple(
        "<http://www.example.com#object_class_1>", "<http://www.example.com#object_property_1>",
        "<http://www.example.com#object_class_2>");

    const std::tuple<std::string, std::string, std::string> data_elements = std::make_tuple(
        "<http://www.example.com#object_class_1>", "<http://www.example.com#data_property_1>",
        "<http://www.some.com#datatype_1>");

    // Define prefixes for RDF object and data
    const std::string object_prefixes = R"(prefix ex: <http://www.example.com#>
prefix otr: <http://www.other.com#>
)";

    const std::string data_prefixes = R"(prefix ex: <http://www.example.com#>
prefix so: <http://www.some.com#>
)";

    const std::optional<double> ntm_coord_value = std::nullopt;

    // Mock adding RDF object and data to triples
    EXPECT_CALL(mock_triple_writer_, addElementObjectToTriple(object_prefixes, object_elements))
        .Times(3);

    EXPECT_CALL(mock_triple_writer_,
                addElementDataToTriple(data_prefixes, data_elements,
                                       message_feature.getNodes().at(0).getValue().value(),
                                       message_feature.getNodes().at(0).getMetadata().getReceived(),
                                       ntm_coord_value))
        .Times(1);

    // Define a dummy Turtle (TTL) output for the RDF triples
    std::string dummy_ttl =
        R"(@prefix ex: <http://www.example.com#> .

ex:Class_1WBY11CF080CH470711
	a ex:Class_1 ;
	ex:hasPart car:Class_2WBY11CF080CH470711 .

car:Observation20181116155027123456789
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:Class_2WBY11CF080CH470711 ;
	sosa:hasSimpleResult "98.6"^^xsd:float ;
	sosa:observedProperty car:Data_Q ;
	sosa:phenomenonTime """"2018-11-16 15:50:27.123456789"^^xsd:dateTime""" .)";

    // Mock generating the triple output and returning the dummy TTL
    EXPECT_CALL(*mock_model_config_, getReasonerSettings())
        .Times(2)
        .WillRepeatedly(
            testing::Return(ReasonerSettings(InferenceEngineType::RDFOX, ReasonerSyntaxType::TURTLE,
                                             std::vector<SchemaType>{SchemaType::VEHICLE}, true)));
    EXPECT_CALL(*mock_model_config_, getOutput()).Times(1).WillOnce(testing::Return("output/"));
    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(ReasonerSyntaxType::TURTLE))
        .Times(1)
        .WillOnce(testing::Return(dummy_ttl));

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(*mock_reasoner_service_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock writing the triple output to a file
    EXPECT_CALL(mock_i_file_handler_,
                writeFile(::testing::_, ::testing::Not(::testing::IsEmpty()), ::testing::Eq(true)))
        .Times(1);

    // Assert that the transformation process does not throw any exceptions
    EXPECT_NO_THROW(triple_assembler_->transformMessageToTriple(message_feature));
}

/**
 * @brief Unit test for transforming a multi-node message to triples with exception handling.
 *
 * This test sets up a message with multiple nodes, including one invalid node to produce an
 * error.
 * It verifies the transformation of the message into triples, ensuring that exceptions
 * are handled correctly during the process and that the transformation completes successfully.
 */
TEST_F(TripleAssemblerUnitTest,
       TransformMessageMultiNodeToTripleWithAnExceptionGeneratingTriplesSuccess) {
    setUpMessage();

    // Add two nodes more to the `message_feature`
    nodes_.emplace_back(
        "Vehicle.FuelLevel", "75", Metadata(),
        std::vector<std::string>{"Vehicle.FuelLevel"});  // Add data point as supported, `Node`
                                                         // validation should not fail for this
                                                         // test
    nodes_.emplace_back(
        "InvalidNode", "error_value", Metadata(),
        std::vector<std::string>{"InvalidNode"});  // Add data point as supported, `Node`
                                                   // validation should not fail for this test

    auto message_header = MessageHeader(VIN, SchemaType::VEHICLE);
    DataMessage message_feature(message_header, nodes_);

    // Mock data store has been setup
    EXPECT_CALL(*mock_reasoner_service_, checkDataStore()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock_triple_writer_,
                initiateTriple(VIN))  // ID is the same from message header id
        .Times(1);

    // Mock the file reader to extract the reasoner query:
    // - Only the first node uses the object query in, it in its name 5 elements
    // - Node one and two read the data query, the third one has throw an error, which should be
    //      logged.

    TripleAssemblerHelper::QueryPair query_pair;
    query_pair.object_property =
        std::make_pair(QueryLanguageType::SPARQL, "MOCK QUERY FOR OBJECTS PROPERTY");
    query_pair.data_property =
        std::make_pair(QueryLanguageType::SPARQL, "MOCK QUERY FOR DATA PROPERTY");
    std::map<SchemaType, TripleAssemblerHelper::QueryPair> query_map = {
        {SchemaType::VEHICLE, query_pair}};
    TripleAssemblerHelper triple_assembler_helper(query_map);

    EXPECT_CALL(*mock_model_config_, getQueriesTripleAssemblerHelper())
        .Times(2)
        .WillRepeatedly(testing::Return(triple_assembler_helper));

    // Mock the SHACL queries responses (first and second node)
    EXPECT_CALL(*mock_reasoner_service_, queryData("MOCK QUERY FOR OBJECTS PROPERTY",
                                                   QueryLanguageType::SPARQL, ::testing::_))
        .Times(3)
        .WillRepeatedly(testing::Return("MOCK RESPONSE FOR OBJECTS PROPERTY"));
    EXPECT_CALL(*mock_reasoner_service_,
                queryData("MOCK QUERY FOR DATA PROPERTY", QueryLanguageType::SPARQL, ::testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return("MOCK RESPONSE FOR DATA PROPERTY"));

    // Mock the data added to the RDF triples (only first node)
    EXPECT_CALL(mock_triple_writer_, addElementObjectToTriple(::testing::_, ::testing::_)).Times(3);

    // Mock the data triple writer adding the values of the correct nodes one and two
    EXPECT_CALL(mock_triple_writer_,
                addElementDataToTriple(::testing::_, ::testing::_, ::testing::Eq("98.6"),
                                       ::testing::_, ::testing::_))
        .Times(1);
    EXPECT_CALL(mock_triple_writer_,
                addElementDataToTriple(::testing::_, ::testing::_, ::testing::Eq("75"),
                                       ::testing::_, ::testing::_))
        .Times(1);

    // Mock generate the triples (only first two nodes)
    std::string dummy_ttl = "some_ttl";

    EXPECT_CALL(*mock_model_config_, getReasonerSettings())
        .Times(2)
        .WillRepeatedly(
            testing::Return(ReasonerSettings(InferenceEngineType::RDFOX, ReasonerSyntaxType::TURTLE,
                                             std::vector<SchemaType>{SchemaType::VEHICLE}, true)));
    EXPECT_CALL(*mock_model_config_, getOutput()).Times(1).WillOnce(testing::Return("output/"));

    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(::testing::_))
        .Times(1)
        .WillOnce(testing::Return(dummy_ttl));

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(*mock_reasoner_service_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock write triple output file (only first two nodes)
    EXPECT_CALL(mock_i_file_handler_, writeFile(::testing::_, ::testing::_, ::testing::Eq(true)))
        .Times(1);

    // Assert
    EXPECT_NO_THROW(triple_assembler_->transformMessageToTriple(message_feature));
}

/**
 * @brief Unit test for initialization failure of the Triple Assembler when no data store is
 * set.
 */
TEST_F(TripleAssemblerUnitTest, InitializeFailWhenAnyDataStoreIsSet) {
    // Mock the data store check to return false, indicating the data store is not available
    EXPECT_CALL(*mock_reasoner_service_, checkDataStore()).WillOnce(testing::Return(false));

    // Assert that the initialization process throws a runtime error due to missing data store
    EXPECT_THROW(triple_assembler_->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for the failure of TripleAssembler initialization when validation shapes are
 empty.
 */
TEST_F(TripleAssemblerUnitTest, InitializeFailIfValidationShapesAreEmptyInTheModelConfig) {
    // Mock functions
    EXPECT_CALL(*mock_reasoner_service_, checkDataStore()).WillOnce(testing::Return(true));

    // Create a mock ModelConfig with an empty list of validation shapes files
    EXPECT_CALL(*mock_model_config_, getValidationShapes())
        .Times(1)
        .WillOnce(testing::Return(
            std::vector<std::pair<ReasonerSyntaxType, std::string>>{}));  // This cannot be empty
                                                                          // for successful
                                                                          // initialization

    // Attempt to initialize the TripleAssembler and expect a runtime error to be thrown
    EXPECT_THROW(triple_assembler_->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for handling failure during the initialization of the TripleAssembler
 *        when loading data to reasoner engine from validation shapes goes wrong.
 */
TEST_F(TripleAssemblerUnitTest, InitializeFailsIfLoadingDataFromValidationShapesGoesWrong) {
    // Mock functions
    EXPECT_CALL(*mock_reasoner_service_, checkDataStore()).WillOnce(testing::Return(true));

    // Mock reading SHACL shape files
    EXPECT_CALL(*mock_model_config_, getValidationShapes())
        .Times(1)
        .WillOnce(testing::Return(std::vector<std::pair<ReasonerSyntaxType, std::string>>{
            {ReasonerSyntaxType::TURTLE, "some_validation_shape_content"}}));

    // Simulate a failure in loading the data, returning false
    EXPECT_CALL(*mock_reasoner_service_,
                loadData(testing::StrEq("some_validation_shape_content"), ::testing::_))
        .WillOnce(testing::Return(false));  // Fail loading data

    // Initialize TripleAssembler and expect a runtime error to be thrown
    EXPECT_THROW(triple_assembler_->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for handling failure when transforming a message to triples
 *        due to the absence of a reasoner data store.
 */
TEST_F(TripleAssemblerUnitTest, TransformMessageToTripleFailsWhenAnyDataStoreIsSet) {
    // Set up the initial message for the test
    setUpMessage();
    auto message_header = MessageHeader(VIN, SchemaType::VEHICLE);
    DataMessage message_feature(message_header, nodes_);

    // Mock the data store check to return false, indicating the data store is not available
    EXPECT_CALL(*mock_reasoner_service_, checkDataStore()).WillOnce(testing::Return(false));

    // Ensure that no other functions are called since the data store is not set up
    EXPECT_CALL(mock_triple_writer_, initiateTriple(::testing::_)).Times(0);
    EXPECT_CALL(*mock_reasoner_service_, queryData(::testing::_, ::testing::_, ::testing::_))
        .Times(0);
    EXPECT_CALL(*mock_model_config_, getQueriesTripleAssemblerHelper()).Times(0);
    EXPECT_CALL(*mock_model_config_, getReasonerSettings()).Times(0);
    EXPECT_CALL(*mock_model_config_, getOutput()).Times(0);
    EXPECT_CALL(mock_triple_writer_, addElementObjectToTriple(::testing::_, ::testing::_)).Times(0);
    EXPECT_CALL(mock_triple_writer_,
                addElementDataToTriple(::testing::_, ::testing::_, ::testing::_, ::testing::_,
                                       ::testing::_))
        .Times(0);
    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(::testing::_)).Times(0);

    // Assert that the transformation process throws a runtime error due to the missing data store
    EXPECT_THROW(triple_assembler_->transformMessageToTriple(message_feature), std::runtime_error);
}

// Testing messages with coordinates

/**
 * @brief Unit test for transforming a message with coordinates pair to RDF triples successfully.
 *
 * This test sets up a message with coordinates and model base, then mocks various components
 * to simulate the transformation of the message into RDF triples. It verifies that the
 * transformation and a double NTM value completes with the expected behaviour without throwing
 * exceptions.
 */

TEST_F(TripleAssemblerUnitTest, TransformMessageToRDFTripleWithCoordinatesSuccess) {
    // Set up the initial message and model base for the test

    std::vector<std::string> supported_data_points = {"Vehicle.CurrentLocation.Latitude",
                                                      "Vehicle.CurrentLocation.Longitude"};

    // Set up the message with coordinates
    nodes_.emplace_back("Vehicle.CurrentLocation.Latitude", "40.0", Metadata(),
                        supported_data_points);
    nodes_.emplace_back("Vehicle.CurrentLocation.Longitude", "50.0", Metadata(),
                        supported_data_points);

    auto message_header = MessageHeader(VIN, SchemaType::VEHICLE);
    DataMessage message_feature(message_header, nodes_);

    // Set up the initial expectations for the test
    initialSetupExpectations(1, 2, 2);

    // Mock adding RDF object and data to triples
    EXPECT_CALL(mock_triple_writer_, addElementObjectToTriple(::testing::_, ::testing::_)).Times(2);

    EXPECT_CALL(mock_triple_writer_,
                addElementDataToTriple(::testing::_, ::testing::_, ::testing::_, ::testing::_,
                                       IsOptionalWithValue()))
        .Times(2);

    // Mock generate the triple output and return a dummy TTL
    std::string dummy_ttl = "some_ttl";
    EXPECT_CALL(*mock_model_config_, getReasonerSettings())
        .Times(2)
        .WillRepeatedly(
            testing::Return(ReasonerSettings(InferenceEngineType::RDFOX, ReasonerSyntaxType::TURTLE,
                                             std::vector<SchemaType>{SchemaType::VEHICLE}, true)));
    EXPECT_CALL(*mock_model_config_, getOutput()).Times(1).WillOnce(testing::Return("output/"));
    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(ReasonerSyntaxType::TURTLE))
        .Times(1)
        .WillOnce(testing::Return(dummy_ttl));

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(*mock_reasoner_service_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock writing the triple output to a file
    EXPECT_CALL(mock_i_file_handler_,
                writeFile(::testing::_, ::testing::Not(::testing::IsEmpty()), ::testing::Eq(true)))
        .Times(1);

    // Assert that the transformation process does not throw any exceptions
    EXPECT_NO_THROW(triple_assembler_->transformMessageToTriple(message_feature));
}

/**
 * @brief Unit test for handling failure when coordinates are given in the message but getting
 the coordinates in NTM returns null option.
 */
TEST_F(TripleAssemblerUnitTest, TransformMessageToTripleFailsWhenCoordinatesAreNull) {
    // Set up the initial message for the test
    setUpMessage();

    std::vector<std::string> supported_data_points = {"Vehicle.CurrentLocation.Latitude",
                                                      "Vehicle.CurrentLocation.Longitude"};

    // Add two nodes more to the `message_feature_` with a wrong coordinate
    nodes_.emplace_back("Vehicle.CurrentLocation.Latitude", "", Metadata(),
                        supported_data_points);  // Invalid value
    nodes_.emplace_back("Vehicle.CurrentLocation.Longitude", "50.0", Metadata(),
                        supported_data_points);

    auto message_header = MessageHeader(VIN, SchemaType::VEHICLE);
    DataMessage message_feature(message_header, nodes_);

    // Set up the initial expectations for the test (only first node, the coordinates nodes are
    // excluded, since latitude do not generate a valid NTM value)
    initialSetupExpectations(1, 3, 1);

    // Mock the data added to the RDF triples (only first node, the coordinates contain an error)
    EXPECT_CALL(mock_triple_writer_, addElementObjectToTriple(::testing::_, ::testing::_)).Times(3);
    EXPECT_CALL(mock_triple_writer_,
                addElementDataToTriple(::testing::_, ::testing::_, ::testing::_, ::testing::_,
                                       ::testing::Eq(std::nullopt)))
        .Times(1);

    // Mock generate the triple output (since the first node is correct) and return a dummy TTL
    std::string dummy_ttl = "some_ttl";
    EXPECT_CALL(*mock_model_config_, getReasonerSettings())
        .Times(2)
        .WillRepeatedly(
            testing::Return(ReasonerSettings(InferenceEngineType::RDFOX, ReasonerSyntaxType::TURTLE,
                                             std::vector<SchemaType>{SchemaType::VEHICLE}, true)));
    EXPECT_CALL(*mock_model_config_, getOutput()).Times(1).WillOnce(testing::Return("output/"));
    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(::testing::_))
        .Times(1)
        .WillRepeatedly(testing::Return(dummy_ttl));

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(*mock_reasoner_service_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock write triple output file (only first node)
    EXPECT_CALL(mock_i_file_handler_, writeFile(::testing::_, ::testing::_, ::testing::Eq(true)))
        .Times(1);

    // Assert
    EXPECT_NO_THROW(triple_assembler_->transformMessageToTriple(message_feature));
}