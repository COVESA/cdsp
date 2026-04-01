#include <gtest/gtest.h>

#include <string>

#include "data_types.h"
#include "random_utils.h"
#include "rdfox_adapter.h"
#include "server_data_fixture.h"

class RDFoxAdapterIntegrationTest : public ::testing::Test {
   protected:
    RDFoxAdapter* adapter_;

    const ReasonerServerData rdfox_server_ = ServerDataFixture::getValidRDFoxServerData();
    const std::string triple_subject_ = RandomUtils::generateRandomString(8);
    const std::string triple_object_ = RandomUtils::generateRandomString(6);

    void SetUp() override {
        adapter_ = new RDFoxAdapter(rdfox_server_);
        adapter_->initialize();

        // Initial load to ensure datastore is accessible
        if (!adapter_->checkDataStore()) {
            FAIL() << "Failed to ensure the data store is set up.";
        }
    }

    void TearDown() override {
        // Delete the test data store after all tests have completed
        ASSERT_TRUE(adapter_->deleteDataStore()) << "Failed to clean up the test data store.";

        // Clean up the adapter instance
        delete adapter_;
    }
};

class RDFoxAdapterDataLoadTest
    : public RDFoxAdapterIntegrationTest,
      public ::testing::WithParamInterface<std::pair<ReasonerSyntaxType, std::string>> {
   protected:
    void SetUp() override {
        // Call the base class setup
        RDFoxAdapterIntegrationTest::SetUp();
    }

    void TearDown() override {
        // Call the base class teardown
        RDFoxAdapterIntegrationTest::TearDown();
    }
};

INSTANTIATE_TEST_SUITE_P(
    DataLoadFormats, RDFoxAdapterDataLoadTest,
    ::testing::Values(std::make_pair(ReasonerSyntaxType::TURTLE, "text_turtle_format"),
                      std::make_pair(ReasonerSyntaxType::TRIG, "application_trig_format"),
                      std::make_pair(ReasonerSyntaxType::NTRIPLES, "application_n_triples_format"),
                      std::make_pair(ReasonerSyntaxType::NQUADS, "application_n_quads_format")),
    [](const ::testing::TestParamInfo<std::pair<ReasonerSyntaxType, std::string>>& info) {
        return info.param.second;  // Test name
    });

/**
 * @brief Generate data in different formats for the given subject and object.
 *
 * @param subject The subject of the triple.
 * @param object The object of the triple.
 * @return A map of data formats and their corresponding data.
 */
std::map<ReasonerSyntaxType, std::string> generateDataFormats(const std::string& subject,
                                                              const std::string& object) {
    std::map<ReasonerSyntaxType, std::string> data_formats;

    // Turtle format
    data_formats[ReasonerSyntaxType::TURTLE] = R"(
        @prefix car: <http://example.com/car#> .
        car:)" + subject + R"( a car:)" + object +
                                               R"( .
    )";

    // TriG format
    data_formats[ReasonerSyntaxType::TRIG] = R"(
        @prefix car: <http://example.com/car#> .
        {
            car:)" + subject + R"( a car:)" + object +
                                             R"( .
        }
    )";

    // N-Triples format
    data_formats[ReasonerSyntaxType::NTRIPLES] =
        R"(
        <http://example.com/car#)" +
        subject +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.com/car#)" + object +
        R"(> .
    )";

    // N-Quads format
    data_formats[ReasonerSyntaxType::NQUADS] =
        R"(
    <http://example.com/car#)" +
        subject +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.com/car#)" + object +
        R"(> .
)";

    return data_formats;
}

/**
 * @brief Test to verify the initialization of the data store in RDFoxAdapter.
 *
 * This test checks the implicit initialization of the data store during the
 * setup phase of RDFoxAdapter. If the data store already exists or is created
 * successfully, no error will be thrown. Additional assertions can be added
 * to verify specific behaviors if needed.
 */
TEST_F(RDFoxAdapterIntegrationTest, DataStoreInitializationTest) { SUCCEED(); }

/**
 * @brief Test to verify the loading of data into the data store in RDFoxAdapter.
 */
TEST_P(RDFoxAdapterDataLoadTest, LoadAndQueryDataConsistency) {
    // Get the data format and content type from the test parameters
    const ReasonerSyntaxType& data_format = GetParam().first;
    const std::string& content_type_str = reasonerSyntaxTypeToContentType(data_format);

    std::cout << "Data format: " << content_type_str << std::endl;

    // Generate data for the format
    auto data_formats = generateDataFormats(triple_subject_, triple_object_);
    std::string data_to_load = data_formats[data_format];

    // Load the data into the data store
    ASSERT_TRUE(adapter_->loadData(data_to_load, content_type_str))
        << "Failed to load data in format: " << content_type_str;

    // Define a SPARQL query to select all triples in the data store
    std::string sparql = "SELECT ?s ?p ?o WHERE { ?s ?p ?o }";

    // Execute the SPARQL query and store the result
    std::string result = adapter_->queryData(sparql);

    // Asserts
    ASSERT_FALSE(result.empty()) << "SPARQL query returned no results";
    EXPECT_TRUE(result.find("<http://example.com/car#" + triple_subject_ + ">") !=
                std::string::npos)
        << "Result does not contain the expected subject.";
    EXPECT_TRUE(result.find("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>") !=
                std::string::npos)
        << "Result does not contain the expected predicate.";
    EXPECT_TRUE(result.find("<http://example.com/car#" + triple_object_ + ">") != std::string::npos)
        << "Result does not contain the expected object.";
}

/**
 * @brief Test to verify the deletion of the data store in RDFoxAdapter.
 *
 * This test checks the existence of a data store, attempts to delete it,
 * and then verifies that it no longer exists. The test ensures that the
 * data store can be successfully deleted and does not persist after deletion.
 */
TEST_F(RDFoxAdapterIntegrationTest, DeleteDataStoreTest) {
    // Asserts
    ASSERT_TRUE(adapter_->checkDataStore()) << "Any data store has been created.";
    ASSERT_TRUE(adapter_->deleteDataStore()) << "Failed to delete the data store.";
    ASSERT_FALSE(adapter_->checkDataStore()) << "The datastore might still exist or be recreated.";
}

TEST_F(RDFoxAdapterIntegrationTest, CheckConnectionSuccess) {
    std::pair<std::string, std::string> connection;
    ASSERT_NO_THROW(connection = adapter_->createConnection())
        << "Failed to create a new connection to the data store.";

    const auto& [connection_id, auth_token] = connection;

    ASSERT_TRUE(adapter_->checkConnection(connection_id)) << "Connection does not exist.";
}

/**
 * @brief Test case for creating a connection using RDFoxAdapter.
 *
 * This test attempts to create a new connection to the data store using the RDFoxAdapter.
 * It verifies that the connection creation is successful by checking that a valid connection ID is
 * returned.
 */
TEST_F(RDFoxAdapterIntegrationTest, CreateConnectionTest) {
    std::string subject_1 = RandomUtils::generateRandomString(8);
    std::string subject_2 = RandomUtils::generateRandomString(8);

    std::string object_name_1 = RandomUtils::generateRandomString(6);
    std::string object_name_2 = RandomUtils::generateRandomString(6);

    std::string object_age = std::to_string(RandomUtils::generateRandomInt(0, 50));

    std::string ttl_data = R"(
        @prefix ex: <http://example.org/> .
        @prefix foaf: <http://xmlns.com/foaf/0.1/> .
        @prefix dct: <http://purl.org/dc/terms/> .

        ex:)" + subject_1 +
                           R"( a foaf:Person ;
            foaf:name ")" + object_name_1 +
                           R"(" ;
            ex:age ")" + object_age +
                           R"("^^xsd:integer .

        ex:)" + subject_2 +
                           R"( a foaf:Person ;
            foaf:name ")" + object_name_2 +
                           R"(" ;
            foaf:knows ex:)" +
                           subject_1 + R"( .
    )";

    // Attempt to load the Turtle data into the data store and assert success
    ASSERT_TRUE(adapter_->loadData(ttl_data)) << "Failed to load Turtle data into the store";

    // Define a SPARQL query to select all triples in the data store
    std::string sparql = "SELECT ?s ?p ?o WHERE { ?s ?p ?o }";

    // Attempt to create a new connection to the data store
    std::pair<std::string, std::string> connection;
    ASSERT_NO_THROW(connection = adapter_->createConnection())
        << "Failed to create a new connection to the data store.";

    const auto& [connection_id, auth_token] = connection;

    ASSERT_TRUE(adapter_->checkConnection(connection_id)) << "Connection does not exist.";

    // Asserts
    ASSERT_FALSE(connection_id.empty()) << "Connection ID is empty.";
    ASSERT_FALSE(auth_token.empty()) << "Authentication token is empty.";

    // Create a cursor for a SPARQL query
    std::string cursor;
    ASSERT_NO_THROW(cursor = adapter_->createCursor(connection_id, auth_token, sparql));
    ASSERT_FALSE(cursor.empty()) << "Failed to create a cursor for the SPARQL query.";

    int limit = 2;  // Number of rows to retrieve per request
    std::string response;

    // Advance the cursor to retrieve the results
    // Pagination 1
    ASSERT_TRUE(adapter_->advanceCursor(connection_id, auth_token, cursor,
                                        DataQueryAcceptType::TEXT_CSV, "open", limit, &response));
    std::string expected_response = "s,p,o\r\nhttp://example.org/" + subject_1 +
                                    ",http://www.w3.org/1999/02/22-rdf-syntax-ns#type"
                                    ",http://xmlns.com/foaf/0.1/Person\r\nhttp://example.org/" +
                                    subject_1 + ",http://xmlns.com/foaf/0.1/name," + object_name_1 +
                                    "\r\n";

    ASSERT_EQ(response, expected_response);

    // Pagination 2
    ASSERT_TRUE(adapter_->advanceCursor(connection_id, auth_token, cursor,
                                        DataQueryAcceptType::SPARQL_JSON, "advance", limit,
                                        &response));
    expected_response =
        "{ \"head\": { \"vars\": [ \"s\", \"p\", \"o\" ] },\n  \"results\": { \"bindings\": [\n    "
        "{ \"s\": { \"type\": \"uri\", \"value\": \"http://example.org/" +
        subject_1 +
        "\" },\n      \"p\": "
        "{ \"type\": \"uri\", \"value\": \"http://example.org/age\" },\n      \"o\": { \"type\": "
        "\"literal\", \"value\": \"" +
        object_age +
        "\", \"datatype\": "
        "\"http://www.w3.org/2001/XMLSchema#integer\" } },\n    { \"s\": { \"type\": \"uri\", "
        "\"value\": \"http://example.org/" +
        subject_2 +
        "\" },\n      \"p\": { \"type\": \"uri\", "
        "\"value\": \"http://www.w3.org/1999/02/22-rdf-syntax-ns#type\" },\n      \"o\": { "
        "\"type\": \"uri\", \"value\": \"http://xmlns.com/foaf/0.1/Person\" } }\n  ] }\n}\n";

    ASSERT_EQ(response, expected_response);
    // Pagination 3
    ASSERT_TRUE(adapter_->advanceCursor(connection_id, auth_token, cursor,
                                        DataQueryAcceptType::TEXT_TSV, "advance", limit,
                                        &response));
    expected_response = "?s\t?p\t?o\n<http://example.org/" + subject_2 +
                        ">\t<http://xmlns.com/foaf/0.1/"
                        "name>\t\"" +
                        object_name_2 + "\"\n<http://example.org/" + subject_2 +
                        ">\t<http://xmlns.com/foaf/0.1/"
                        "knows>\t<http://example.org/" +
                        subject_1 + ">\n";

    ASSERT_EQ(response, expected_response);

    // Pagination 4
    ASSERT_TRUE(adapter_->advanceCursor(connection_id, auth_token, cursor,
                                        DataQueryAcceptType::SPARQL_XML, "advance", limit,
                                        &response));
    expected_response =
        "<?xml version=\"1.0\"?>\n<sparql "
        "xmlns=\"http://www.w3.org/2005/sparql-results#\">\n<head>\n  <variable name=\"s\"/>\n  "
        "<variable name=\"p\"/>\n  <variable name=\"o\"/>\n</head>\n<results/>\n</sparql>\n";

    ASSERT_EQ(response, expected_response);

    ASSERT_TRUE(adapter_->deleteCursor(connection_id, cursor));
}

/**
 * @brief Test case loading data with an unsupported format.
 */
TEST_F(RDFoxAdapterIntegrationTest, LoadDataUnsupportedFormat) {
    std::string invalid_data = R"(
        {
            "subject": "test",
            "predicate": "hasValue",
            "object": "value"
        }
    )";

    // Attempt to load the data into the data store
    ASSERT_FALSE(adapter_->loadData(invalid_data, "invalid_format"))
        << "Unexpected: Data was loaded with an unsupported format.";
}
