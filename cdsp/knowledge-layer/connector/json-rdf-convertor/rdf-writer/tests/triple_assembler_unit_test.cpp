#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "data_types.h"
#include "mock_adapter.h"
#include "rdfox_adapter.h"
#include "triple_assembler.h"
#include "vin_utils.h"

struct TestParamsTransformMessageToRDFTriple {
    DataMessage message;
    std::vector<std::string> query_object;
    std::string query_data;
    std::vector<std::string> object_triple_result;
    std::string data_triple_result;
};

class MockFileHandler : public IFileHandler {
   public:
    MOCK_METHOD(std::string, readFile, (const std::string& file_path), (override));
    MOCK_METHOD(void, writeFile,
                (const std::string& file_path, const std::string& content, bool append_data),
                (override));
};

class MockTripleWriter : public TripleWriter {
   public:
    MOCK_METHOD(void, initiateTriple, (const std::string&), (override));
    MOCK_METHOD(void, addRDFObjectToTriple,
                (const std::string&, (const std::tuple<std::string, std::string, std::string>&) ),
                (override));

    MOCK_METHOD(void, addRDFDataToTriple,
                ((const std::string&), (const std::tuple<std::string, std::string, std::string>&),
                 (const std::string&), (const std::string&), (const std::optional<double>&) ),
                (override));
    MOCK_METHOD(std::string, generateTripleOutput, (const RDFSyntaxType&), (override));
};

class TripleAssemblerUnitTest : public ::testing::Test {
   protected:
    const std::string VIN = VinUtils::getRandomVinString();

    TripleAssembler* triple_assembler;
    DataMessage message_feature_;

    // Set up mock RDFoxAdapter
    MockAdapter mock_adapter_{"localhost", "8080", "test_auth", "test_ds"};
    MockFileHandler mock_file_handler_;
    MockTripleWriter mock_triple_writer_;

    // Create a mock ModelConfig
    ModelConfig model_config_;

    void SetUp() override {
        // Mock functions to be call and expectations
        triple_assembler = new TripleAssembler(model_config_, mock_adapter_, mock_file_handler_,
                                               mock_triple_writer_);
    }

    void TearDown() override { delete triple_assembler; }

    void setUpModelBase() {
        model_config_.shacl_shapes_files = {"shacl_shape1.ttl", "shacl_shape2.ttl"};
        model_config_.triple_assembler_queries_files = {
            {"vss", {"data_property.rq", "object_property.rq"}}};
        model_config_.reasoner_settings.output_format = RDFSyntaxType::TURTLE;
        model_config_.output_file_path = "/outputs/triple_assembler_unit_test/";
    }

    void setUpMessageHeader(const std::string& date_time) {
        message_feature_.header.id = VIN;
        message_feature_.header.tree = "VSS";
        message_feature_.header.date_time = date_time;
    }

    void setUpMessage() {
        setUpMessageHeader("2021-09-01T12:00:00Z");

        Node node_1;
        node_1.name = "Vehicle.Powertrain.TractionBattery.StateOfCharge.CurrentEnergy";
        node_1.value = "98.6";

        message_feature_.nodes.push_back(node_1);
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
                                  const std::string& query_object_response = "",
                                  const std::string& query_data_response = "") {
        // Mock data store has been setup
        EXPECT_CALL(mock_adapter_, checkDataStore())
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

        // Mock the file reader to extract the SHACL query
        EXPECT_CALL(mock_file_handler_, readFile(::testing::StrEq("object_property.rq")))
            .Times(times_executing_object_related_functions)
            .WillRepeatedly(::testing::Return(query_object));

        EXPECT_CALL(mock_file_handler_, readFile(::testing::StrEq("data_property.rq")))
            .Times(times_executing_data_related_functions)
            .WillRepeatedly(::testing::Return(query_data));

        // Mock querying data using the SHACL queries and returning predefined responses
        EXPECT_CALL(mock_adapter_, queryData(query_object, ::testing::StrEq("table/csv")))
            .Times(times_executing_object_related_functions)
            .WillRepeatedly(testing::Return(query_object_response));

        EXPECT_CALL(mock_adapter_, queryData(query_data, ::testing::StrEq("table/csv")))
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
    // Set up the model base for the test
    setUpModelBase();

    // Mock the data store check to return true, indicating the data store is available
    EXPECT_CALL(mock_adapter_, checkDataStore()).WillOnce(testing::Return(true));

    // Mock reading SHACL shape files and returning predefined data
    EXPECT_CALL(mock_file_handler_, readFile(testing::StrEq("shacl_shape1.ttl")))
        .WillOnce(testing::Return("data1"));
    EXPECT_CALL(mock_file_handler_, readFile(testing::StrEq("shacl_shape2.ttl")))
        .WillOnce(testing::Return("data2"));

    // Mock loading data into the adapter and returning success
    EXPECT_CALL(mock_adapter_, loadData(testing::StrEq("data1"), ::testing::_))
        .WillOnce(testing::Return(true));
    EXPECT_CALL(mock_adapter_, loadData(testing::StrEq("data2"), ::testing::_))
        .WillOnce(testing::Return(true));

    // Assert that the initialization process does not throw any exceptions
    EXPECT_NO_THROW(triple_assembler->initialize());
}

/**
 * @brief Unit test for transforming a message to RDF triples successfully.
 *
 * This test sets up a message and model base, then mocks various components
 * to simulate the transformation of a message into RDF triples. It verifies
 * that the transformation process completes with the expected behaviour without throwing
 * exceptions.
 */
TEST_F(TripleAssemblerUnitTest, TransformMessageToRDFTripleSuccess) {
    // Set up the initial message and model base for the test
    setUpMessage();
    setUpModelBase();

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
    EXPECT_CALL(mock_triple_writer_, addRDFObjectToTriple(object_prefixes, object_elements))
        .Times(3);

    EXPECT_CALL(mock_triple_writer_,
                addRDFDataToTriple(data_prefixes, data_elements, message_feature_.nodes.at(0).value,
                                   message_feature_.header.date_time, ntm_coord_value))
        .Times(1);

    // Define a dummy Turtle (TTL) output for the RDF triples
    std::string dummy_ttl =
        R"(@prefix ex: <http://www.example.com#> .

ex:Class_1WBY11CF080CH470711
	a ex:Class_1 ;
	ex:hasPart car:Class_2WBY11CF080CH470711 .

car:Observation20181116155027O0
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:Class_2WBY11CF080CH470711 ;
	sosa:hasSimpleResult "98.6"^^xsd:float ;
	sosa:observedProperty car:Data_Q ;
	sosa:phenomenonTime """"2018-11-16 15:50:27"^^xsd:dateTime""" .)";

    // Mock generating the triple output and returning the dummy TTL
    EXPECT_CALL(mock_triple_writer_,
                generateTripleOutput(model_config_.reasoner_settings.output_format))
        .Times(1)
        .WillOnce(testing::Return(dummy_ttl));

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(mock_adapter_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock writing the triple output to a file
    EXPECT_CALL(mock_file_handler_,
                writeFile(::testing::_, ::testing::Not(::testing::IsEmpty()), ::testing::Eq(true)))
        .Times(1);

    // Assert that the transformation process does not throw any exceptions
    EXPECT_NO_THROW(triple_assembler->transformMessageToRDFTriple(message_feature_));
}

/**
 * @brief Unit test for transforming a multi-node message to RDF triples with exception handling.
 *
 * This test sets up a message with multiple nodes, including one invalid node to produces an error.
 * It verifies the transformation of the message into RDF triples, ensuring that exceptions
 * are handled correctly during the process and that the transformation completes successfully.
 */
TEST_F(TripleAssemblerUnitTest,
       TransformMessageMultiNodeToRDFTripleWithAnExceptionGeneratingTriplesSuccess) {
    setUpMessage();
    setUpModelBase();

    // Add two nodes more to the `message_feature_`

    Node node_2;
    node_2.name = "Vehicle.FuelLevel";
    node_2.value = "75";
    message_feature_.nodes.push_back(node_2);

    Node node_3;
    node_3.name = "InvalidNode";  // Simulate an error for this node
    node_3.value = "error_value";
    message_feature_.nodes.push_back(node_3);

    // Mock data store has been setup
    EXPECT_CALL(mock_adapter_, checkDataStore()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock_triple_writer_,
                initiateTriple(VIN))  // ID is the same from message header id
        .Times(1);

    // Mock the file reader to extract the SHACL query
    // - Only the first node uses the object query in, it in its name 5 elements
    EXPECT_CALL(mock_file_handler_, readFile(::testing::StrEq("object_property.rq")))
        .Times(3)
        .WillRepeatedly(::testing::Return("MOCK QUERY FOR OBJECTS PROPERTY"));
    // - Node one and two read the data query, the third one has throw an error, which should be
    //      logged.
    EXPECT_CALL(mock_file_handler_, readFile(::testing::StrEq("data_property.rq")))
        .Times(2)
        .WillRepeatedly(::testing::Return("MOCK QUERY FOR DATA PROPERTY"));

    // Mock the SHACL queries responses (first and second node)
    EXPECT_CALL(mock_adapter_, queryData("MOCK QUERY FOR OBJECTS PROPERTY", ::testing::_)).Times(3);
    EXPECT_CALL(mock_adapter_, queryData("MOCK QUERY FOR DATA PROPERTY", ::testing::_)).Times(2);

    // Mock the data added to the RDF triples (only first node)
    EXPECT_CALL(mock_triple_writer_, addRDFObjectToTriple(::testing::_, ::testing::_)).Times(3);

    // Mock the data triple writer adding the values of the correct nodes one and two
    EXPECT_CALL(mock_triple_writer_,
                addRDFDataToTriple(::testing::_, ::testing::_, ::testing::Eq("98.6"), ::testing::_,
                                   ::testing::_))
        .Times(1);
    EXPECT_CALL(mock_triple_writer_,
                addRDFDataToTriple(::testing::_, ::testing::_, ::testing::Eq("75"), ::testing::_,
                                   ::testing::_))
        .Times(1);

    // Mock generate the triples (only first two nodes)
    std::string dummy_ttl = "some_ttl";

    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(::testing::_))
        .Times(1)
        .WillRepeatedly(testing::Return(dummy_ttl));
    ;

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(mock_adapter_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock write triple output file (only first two nodes)
    EXPECT_CALL(mock_file_handler_, writeFile(::testing::_, ::testing::_, ::testing::Eq(true)))
        .Times(1);

    // Assert
    EXPECT_NO_THROW(triple_assembler->transformMessageToRDFTriple(message_feature_));
}

/**
 * @brief Unit test for initialization failure of the Triple Assembler when no RDF data store is
 * set.
 */
TEST_F(TripleAssemblerUnitTest, InitializeFailWhenAnyRDFDataStoreIsSet) {
    // Mock the data store check to return false, indicating the data store is not available
    EXPECT_CALL(mock_adapter_, checkDataStore()).WillOnce(testing::Return(false));

    // Ensure that no file reading operations are attempted
    EXPECT_CALL(mock_file_handler_, readFile(testing::_)).Times(0);

    // Assert that the initialization process throws a runtime error due to missing data store
    EXPECT_THROW(triple_assembler->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for the failure of TripleAssembler initialization when SHACL shapes are empty.
 */
TEST_F(TripleAssemblerUnitTest, InitializeFailIfShaclShapesAreEmptyInTheModelConfig) {
    // Create a mock ModelConfig with an empty list of SHACL shapes files
    model_config_.shacl_shapes_files = {};  // This cannot be empty for successful initialization

    // Mock functions
    EXPECT_CALL(mock_adapter_, checkDataStore()).WillOnce(testing::Return(true));

    // Expect no file reads to occur since SHACL shapes are empty
    EXPECT_CALL(mock_file_handler_, readFile(testing::_)).Times(0);

    // Attempt to initialize the TripleAssembler and expect a runtime error to be thrown
    EXPECT_THROW(triple_assembler->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for initializing the TripleAssembler with empty SHACL shape content.
 */
TEST_F(TripleAssemblerUnitTest, InitializeFailIfShaclShapesContentIsEmpty) {
    // Create a mock ModelConfig with a SHACL shape file
    model_config_.shacl_shapes_files = {"shacl_shape.ttl"};

    // Mock functions
    EXPECT_CALL(mock_adapter_, checkDataStore()).WillOnce(testing::Return(true));

    // Simulate reading the SHACL shape file and returning empty content
    EXPECT_CALL(mock_file_handler_, readFile(testing::StrEq("shacl_shape.ttl")))
        .WillOnce(testing::Return(""));  // The content cannot be empty

    // Ensure no data loading is attempted with empty content
    EXPECT_CALL(mock_adapter_, loadData(testing::_, ::testing::_)).Times(0);

    // Attempt to initialize the TripleAssembler and expect a runtime error
    TripleAssembler assembler(model_config_, mock_adapter_, mock_file_handler_,
                              mock_triple_writer_);
    EXPECT_THROW(triple_assembler->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for handling failure during the initialization of the TripleAssembler
 *        when loading data from SHACL shapes goes wrong.
 */
TEST_F(TripleAssemblerUnitTest, InitializeFailsIfLoadingDataFromShaclShapesGoesWrong) {
    // Create a mock ModelConfig with a SHACL shape file
    model_config_.shacl_shapes_files = {"shacl_shape.ttl"};

    // Mock functions
    EXPECT_CALL(mock_adapter_, checkDataStore()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock_file_handler_, readFile(testing::StrEq("shacl_shape.ttl")))
        .WillOnce(testing::Return("data"));

    // Simulate a failure in loading the data, returning false
    EXPECT_CALL(mock_adapter_, loadData(testing::StrEq("data"), ::testing::_))
        .WillOnce(testing::Return(false));  // Fail loading data

    // Initialize TripleAssembler and expect a runtime error to be thrown
    TripleAssembler assembler(model_config_, mock_adapter_, mock_file_handler_,
                              mock_triple_writer_);
    EXPECT_THROW(triple_assembler->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for handling failure when transforming a message to RDF triples
 *        due to the absence of an RDF data store.
 */
TEST_F(TripleAssemblerUnitTest, TransformMessageToRDFTripleFailsWhenAnyRDFDataStoreIsSet) {
    // Set up the initial message for the test
    setUpMessage();

    // Mock the data store check to return false, indicating the data store is not available
    EXPECT_CALL(mock_adapter_, checkDataStore()).WillOnce(testing::Return(false));

    // Ensure that no other functions are called since the data store is not set up
    EXPECT_CALL(mock_triple_writer_, initiateTriple(::testing::_)).Times(0);
    EXPECT_CALL(mock_file_handler_, readFile(::testing::_)).Times(0);
    EXPECT_CALL(mock_file_handler_, writeFile(::testing::_, ::testing::_, ::testing::_)).Times(0);
    EXPECT_CALL(mock_adapter_, queryData(::testing::_, ::testing::_)).Times(0);
    EXPECT_CALL(mock_triple_writer_, addRDFObjectToTriple(::testing::_, ::testing::_)).Times(0);
    EXPECT_CALL(mock_triple_writer_, addRDFDataToTriple(::testing::_, ::testing::_, ::testing::_,
                                                        ::testing::_, ::testing::_))
        .Times(0);
    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(::testing::_)).Times(0);

    // Assert that the transformation process throws a runtime error due to the missing data store
    EXPECT_THROW(triple_assembler->transformMessageToRDFTriple(message_feature_),
                 std::runtime_error);
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
    setUpMessageHeader("2021-09-01T12:00:00Z");
    setUpModelBase();

    Node node_1;
    node_1.name = "Vehicle.CurrentLocation.Latitude";
    node_1.value = "40.0";
    message_feature_.nodes.push_back(node_1);

    Node node_2;
    node_2.name = "Vehicle.CurrentLocation.Longitude";
    node_2.value = "50.0";
    message_feature_.nodes.push_back(node_2);

    // Set up the initial expectations for the test
    initialSetupExpectations(1, 2, 2);

    // Mock adding RDF object and data to triples
    EXPECT_CALL(mock_triple_writer_, addRDFObjectToTriple(::testing::_, ::testing::_)).Times(2);

    EXPECT_CALL(mock_triple_writer_, addRDFDataToTriple(::testing::_, ::testing::_, ::testing::_,
                                                        ::testing::_, IsOptionalWithValue()))
        .Times(2);

    // Mock generate the triple output and return a dummy TTL
    std::string dummy_ttl = "some_ttl";

    EXPECT_CALL(mock_triple_writer_,
                generateTripleOutput(model_config_.reasoner_settings.output_format))
        .Times(1)
        .WillRepeatedly(testing::Return(dummy_ttl));

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(mock_adapter_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock writing the triple output to a file
    EXPECT_CALL(mock_file_handler_,
                writeFile(::testing::_, ::testing::Not(::testing::IsEmpty()), ::testing::Eq(true)))
        .Times(1);

    // Assert that the transformation process does not throw any exceptions
    EXPECT_NO_THROW(triple_assembler->transformMessageToRDFTriple(message_feature_));
}

/**
 * @brief Unit test for handling failure when coordinates are given in the message but getting the
 * coordinates in NTM returns null option.
 */
TEST_F(TripleAssemblerUnitTest, TransformMessageToRDFTripleFailsWhenCoordinatesAreNull) {
    // Set up the initial message for the test
    setUpMessage();
    setUpModelBase();

    // Add two nodes more to the `message_feature_` with a wrong coordinate
    Node node_2;
    node_2.name = "Vehicle.CurrentLocation.Latitude";
    node_2.value = "";  // Invalid value, it produces null option when converting to NTM
    message_feature_.nodes.push_back(node_2);

    Node node_3;
    node_3.name = "Vehicle.CurrentLocation.Longitude";
    node_3.value = "50.0";
    message_feature_.nodes.push_back(node_3);

    // Set up the initial expectations for the test (only first node, the coordinates nodes are
    // excluded, since latitude do not generate a valid NTM value)
    initialSetupExpectations(1, 3, 1);

    // Mock the data added to the RDF triples (only first node, the coordinates contain an error)
    EXPECT_CALL(mock_triple_writer_, addRDFObjectToTriple(::testing::_, ::testing::_)).Times(3);
    EXPECT_CALL(mock_triple_writer_, addRDFDataToTriple(::testing::_, ::testing::_, ::testing::_,
                                                        ::testing::_, ::testing::Eq(std::nullopt)))
        .Times(1);

    // Mock generate the triple output (since the first node is correct) and return a dummy TTL
    std::string dummy_ttl = "some_ttl";
    EXPECT_CALL(mock_triple_writer_, generateTripleOutput(::testing::_))
        .Times(1)
        .WillRepeatedly(testing::Return(dummy_ttl));

    // Mock logging the generated TTL triples to RDFox
    EXPECT_CALL(mock_adapter_, loadData(::testing::StrEq(dummy_ttl), ::testing::_))
        .Times(1)
        .WillOnce(testing::Return(true));

    // Mock write triple output file (only first node)
    EXPECT_CALL(mock_file_handler_, writeFile(::testing::_, ::testing::_, ::testing::Eq(true)))
        .Times(1);

    // Assert
    EXPECT_NO_THROW(triple_assembler->transformMessageToRDFTriple(message_feature_));
}