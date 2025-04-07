#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>

#include "mock_rdfox_adapter.h"
#include "mock_request_builder.h"
#include "random_utils.h"
#include "rdfox_adapter.h"

class RDFoxAdapterTest : public ::testing::Test {
   protected:
    const std::string DATASTORE = "ds-test";
    const ReasonerServerData server_data_{"localhost", "8080", "auth", DATASTORE};

    void SetUp() override {
        mock_rdfox_adapter_ = std::make_shared<MockRDFoxAdapter>(server_data_);
        mock_request_builder_ = std::make_unique<MockRequestBuilder>(
            server_data_.host, server_data_.port, server_data_.auth_base64);
    }

    std::shared_ptr<MockRDFoxAdapter> mock_rdfox_adapter_;
    std::unique_ptr<MockRequestBuilder> mock_request_builder_;
};

/**
 * @brief Test fixture for RDFoxAdapter to check the existence of a datastore.
 *
 * This test verifies that the RDFoxAdapter correctly constructs a request
 * to check the existence of a datastore and receives the expected response.
 */

TEST_F(RDFoxAdapterTest, CheckExistentDatastore) {
    const std::string target = "/datastores";
    const std::string expected_response = "datastore ds-test";

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock `createRequestBuilder` to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::get))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAcceptType("text/csv; charset=UTF-8"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, testing::NotNull()))
        .WillOnce(
            testing::DoAll(testing::SetArgPointee<1>(expected_response), testing::Return(true)));

    // Assert that the datastore exists
    EXPECT_TRUE(mock_rdfox_adapter_->RDFoxAdapter::checkDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the initialization process creates a datastore.
 */
TEST_F(RDFoxAdapterTest, InitializationCreatesDatastore) {
    const std::string target_create_data = "/datastores/" + DATASTORE;

    // Mock check for the existence of the datastore to return false
    EXPECT_CALL(*mock_rdfox_adapter_, checkDataStore()).WillOnce(testing::Return(false));

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target_create_data))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/json"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, nullptr))
        .WillOnce(testing::Return(true));

    // Expect no exception when the adapter initializes
    EXPECT_NO_THROW(mock_rdfox_adapter_->RDFoxAdapter::initialize());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior during initialization
 * when a datastore already exists.
 */
TEST_F(RDFoxAdapterTest, InitializationWithExistingDatastore) {
    // Mock check for the existence of the datastore to return true
    EXPECT_CALL(*mock_rdfox_adapter_, checkDataStore()).WillOnce(testing::Return(true));

    // Ensure that any request to create the datastore is not made.
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder()).Times(0);

    // Expect no exception when the adapter initializes
    EXPECT_NO_THROW(mock_rdfox_adapter_->RDFoxAdapter::initialize());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when loading data into a datastore.
 */
TEST_F(RDFoxAdapterTest, LoadDataSuccess) {
    const std::string target = "/datastores/" + DATASTORE + "/content";
    const std::string ttl_data = "@prefix : <http://example.org/> . :test a :Entity .";
    const ReasonerSyntaxType content_type = ReasonerSyntaxType::TURTLE;

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr,
                setContentType(reasonerSyntaxTypeToContentType(content_type)))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(ttl_data))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, nullptr))
        .WillOnce(testing::Return(true));

    // Expect the data to be loaded successfully
    EXPECT_TRUE(mock_rdfox_adapter_->RDFoxAdapter::loadData(
        ttl_data, reasonerSyntaxTypeToContentType(content_type)));
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful data querying.
 */
TEST_F(RDFoxAdapterTest, QueryDataSuccess) {
    const std::string target = "/datastores/" + DATASTORE + "/sparql";
    const std::string sparql_query = "SELECT ?s WHERE { ?s ?p ?o . }";
    const std::string mock_response = "<http://example.org/test>";

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/sparql-query"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(sparql_query))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr,
                setAcceptType(queryAcceptTypeToString(DataQueryAcceptType::TEXT_TSV)))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, testing::NotNull()))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(mock_response), testing::Return(true)));

    // Expect the response to match the mock response
    EXPECT_EQ(mock_rdfox_adapter_->RDFoxAdapter::queryData(sparql_query, QueryLanguageType::SPARQL,
                                                           DataQueryAcceptType::TEXT_TSV),
              mock_response);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful deletion of a datastore.
 */
TEST_F(RDFoxAdapterTest, DeleteDataStoreSuccess) {
    const std::string target = "/datastores/" + DATASTORE;

    // Mock check for the existence of the datastore to return true
    EXPECT_CALL(*mock_rdfox_adapter_, checkDataStore()).WillOnce(testing::Return(true));

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::delete_))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, nullptr))
        .WillOnce(testing::Return(true));

    // Expect the datastore to be deleted successfully
    EXPECT_TRUE(mock_rdfox_adapter_->deleteDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when attempting to delete a
 nonexistent
 * datastore.
 */
TEST_F(RDFoxAdapterTest, DeleteNonexistentDataStoreSuccess) {
    const std::string target = "/datastores/" + DATASTORE;

    // Mock check for the existence of the datastore to return false
    EXPECT_CALL(*mock_rdfox_adapter_, checkDataStore()).WillOnce(testing::Return(false));

    // Ensure that the query to delete the datastore is not made.
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder()).Times(0);

    // Expect the function to return true when the datastore does not exist
    EXPECT_TRUE(mock_rdfox_adapter_->deleteDataStore());
}

// Unit tests for RDFoxAdapter to verify cursor-related operations

/**
 * @brief Unit test for RDFoxAdapter to verify successful connection creation.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionSuccess) {
    const std::string target = "/datastores/" + DATASTORE + "/connections";
    const std::string random_connection_id =
        std::to_string(RandomUtils::generateRandomInt(0, 1000));
    const std::string random_auth = RandomUtils::generateRandomString(8);
    const std::map<std::string, std::string> response_headers = {
        {"Location", "/datastores/datastore/connections/" + random_connection_id},
        {"RDFox-Authentication-Token", random_auth}};

    // Get the raw pointer to the mock_request_builder_
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/json"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::NotNull(), ::testing::IsNull()))
        .WillOnce(
            testing::DoAll(testing::SetArgPointee<0>(response_headers), testing::Return(true)));

    // Call the function under test
    auto connection = mock_rdfox_adapter_->RDFoxAdapter::createConnection();

    // Assert the results
    EXPECT_EQ(connection.first, random_connection_id);
    EXPECT_EQ(connection.second, "RDFox " + random_auth);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful cursor creation.
 */
TEST_F(RDFoxAdapterTest, CreateCursorSuccess) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string valid_query = RandomUtils::generateRandomString(20);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const std::string target =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors";
    const std::string location_header =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors/" + cursor_id;

    // Mock the headers for the request
    const std::map<std::string, std::string> response_headers = {{"Location", location_header}};

    // Get the raw pointer to the mock_request_builder_
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    ASSERT_TRUE(mock_request_builder_);

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/sparql-query"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAuthorization(auth_token))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(valid_query))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::NotNull(), ::testing::IsNull()))
        .WillOnce(
            testing::DoAll(testing::SetArgPointee<0>(response_headers), testing::Return(true)));

    // Call the function under test
    std::string created_cursor_id;
    ASSERT_NO_THROW(created_cursor_id = mock_rdfox_adapter_->RDFoxAdapter::createCursor(
                        connection_id, auth_token, valid_query));
    EXPECT_EQ(created_cursor_id, cursor_id);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful cursor advance.
 */
TEST_F(RDFoxAdapterTest, AdvanceCursorSuccess) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const DataQueryAcceptType accept_type = DataQueryAcceptType::SPARQL_JSON;
    const std::string response_body = RandomUtils::generateRandomString(20);
    const std::pair<std::string, std::string> valid_operations = {"open", "advance"};
    const std::string operation = RandomUtils::generateRandomInt(0, 1) == 0
                                      ? valid_operations.first
                                      : valid_operations.second;
    const int limit = RandomUtils::generateRandomInt(1, 100);
    const std::string target = "/datastores/" + DATASTORE + "/connections/" + connection_id +
                               "/cursors/" + cursor_id + "?operation=" + operation +
                               "&limit=" + std::to_string(limit);

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::patch))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAuthorization(auth_token))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAcceptType(queryAcceptTypeToString(accept_type)))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::IsNull(), ::testing::NotNull()))
        .WillOnce(testing::DoAll(testing::SetArgPointee<1>(response_body), testing::Return(true)));

    // Call the function under test
    std::string response;
    bool success = mock_rdfox_adapter_->RDFoxAdapter::advanceCursor(
        connection_id, auth_token, cursor_id, accept_type, operation, limit, &response);

    // Assertions
    EXPECT_TRUE(success);
    EXPECT_EQ(response, response_body);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful cursor deletion.
 */
TEST_F(RDFoxAdapterTest, DeleteCursorSuccess) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const std::string target =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors/" + cursor_id;

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::delete_))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::IsNull(), ::testing::IsNull()))
        .WillOnce(testing::Return(true));

    // Call the function under test
    bool success = mock_rdfox_adapter_->RDFoxAdapter::deleteCursor(connection_id, cursor_id);

    // Assertions
    EXPECT_TRUE(success);
}

// Unit tests for RDFoxAdapter to verify error handling

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when datastore creation fails.
 */
TEST_F(RDFoxAdapterTest, FailedToCreateDataStore) {
    const std::string target_create_data = "/datastores/" + DATASTORE;

    // Mock check for the existence of the datastore to return false
    EXPECT_CALL(*mock_rdfox_adapter_, checkDataStore()).WillOnce(testing::Return(false));

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target_create_data))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/json"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, nullptr))
        .WillOnce(testing::Return(false));

    // Expect exception when the adapter initializes
    EXPECT_THROW(mock_rdfox_adapter_->RDFoxAdapter::initialize(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when loading data fails.
 */
TEST_F(RDFoxAdapterTest, LoadDataFailure) {
    const std::string target = "/datastores/" + DATASTORE + "/content";
    const std::string ttl_data = "@prefix : <http://example.org/> . :test a :Entity .";
    const ReasonerSyntaxType content_type = ReasonerSyntaxType::TURTLE;

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr,
                setContentType(reasonerSyntaxTypeToContentType(content_type)))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(ttl_data))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, nullptr))
        .WillOnce(testing::Return(false));

    // Expect false dating data
    EXPECT_FALSE(mock_rdfox_adapter_->RDFoxAdapter::loadData(
        ttl_data, reasonerSyntaxTypeToContentType(content_type)));
}

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when a SPARQL query fails.
 */
TEST_F(RDFoxAdapterTest, QueryDataFailure) {
    const std::string target = "/datastores/" + DATASTORE + "/sparql";
    const std::string sparql_query = "SELECT ?s WHERE { ?s ?p ?o . }";
    const DataQueryAcceptType accept_type = DataQueryAcceptType::TEXT_TSV;

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/sparql-query"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(sparql_query))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAcceptType(queryAcceptTypeToString(accept_type)))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, testing::NotNull()))
        .WillOnce(testing::Return(false));

    // Expect the empty response to match the mock response
    EXPECT_EQ(mock_rdfox_adapter_->RDFoxAdapter::queryData(sparql_query, QueryLanguageType::SPARQL,
                                                           accept_type),
              "");
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when attempting to delete a
 * datastore fails.
 */
TEST_F(RDFoxAdapterTest, DeleteDataStoreFailure) {
    const std::string target = "/datastores/" + DATASTORE;

    // Mock check for the existence of the datastore to return true
    EXPECT_CALL(*mock_rdfox_adapter_, checkDataStore()).WillOnce(testing::Return(true));

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the shared_ptr to MockRequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for MockRequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::delete_))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(nullptr, nullptr))
        .WillOnce(testing::Return(false));

    // Expect the query to delete the datastore to fail
    EXPECT_FALSE(mock_rdfox_adapter_->deleteDataStore());
}

// Unit tests for RDFoxAdapter to verify cursor-related operations

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when a connection cannot be created.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionFailure) {
    const std::string target = "/datastores/" + DATASTORE + "/connections";

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/json"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));

    // Mock the headers for the request and simulate failure
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(testing::NotNull(), nullptr))
        .WillOnce(testing::Return(false));

    // Assert that the function returns an empty optional on failure
    EXPECT_THROW(mock_rdfox_adapter_->createConnection(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when header values are missing after
 * create connection.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionMissingHeader) {
    const std::string target = "/datastores/" + DATASTORE + "/connections";
    const std::string random_connection_id =
        std::to_string(RandomUtils::generateRandomInt(0, 1000));
    const std::string random_header_value = RandomUtils::generateRandomString(8);
    const std::pair<std::string, std::string> valid_headers = {"Location",
                                                               "RDFox-Authentication-Token"};
    const std::string header =
        RandomUtils::generateRandomInt(0, 1) == 0 ? valid_headers.first : valid_headers.second;
    const std::map<std::string, std::string> invalid_response_headers = {
        {header, random_header_value}};  // Missing header required values

    // Get the raw pointer to the mock_request_builder_
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    ASSERT_TRUE(mock_request_builder_);

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/json"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::NotNull(), ::testing::IsNull()))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(invalid_response_headers),
                                 testing::Return(true)));

    // Expect exception when the location header is missing
    EXPECT_THROW(mock_rdfox_adapter_->createConnection(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when the location header format is
 * incorrect after creating a connection.
 */
TEST_F(RDFoxAdapterTest, CreateConnectionWrongLocationHeaderFormat) {
    const std::string target = "/datastores/" + DATASTORE + "/connections";
    const std::string random_auth = RandomUtils::generateRandomString(8);
    const std::map<std::string, std::string> response_headers = {
        {"Location", "wrong_format_without_slashes"}, {"RDFox-Authentication-Token", random_auth}};

    // Get the raw pointer to the mock_request_builder_
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/json"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::NotNull(), ::testing::IsNull()))
        .WillOnce(
            testing::DoAll(testing::SetArgPointee<0>(response_headers), testing::Return(true)));

    // Expect exception when the location header is missing
    EXPECT_THROW(mock_rdfox_adapter_->createConnection(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when a cursor cannot be created.
 */
TEST_F(RDFoxAdapterTest, CreateCursorFailure) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string invalid_query = RandomUtils::generateRandomString(20);
    const std::string target =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors";

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/sparql-query"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAuthorization(auth_token))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(invalid_query))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::NotNull(), ::testing::IsNull()))
        .WillOnce(testing::Return(false));

    // Expect exception when the cursor creation fails
    EXPECT_THROW(mock_rdfox_adapter_->createCursor(connection_id, auth_token, invalid_query),
                 std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when location header is missing after
 * cursor creation.
 */
TEST_F(RDFoxAdapterTest, CreateCursorMissingLocationHeader) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string valid_query = RandomUtils::generateRandomString(20);
    const std::string target =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors";

    // Mock the headers for the request
    std::map<std::string, std::string> response_headers = {};

    // Get the raw pointer to the mock_request_builder_
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    ASSERT_TRUE(mock_request_builder_);

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/sparql-query"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAuthorization(auth_token))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(valid_query))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::NotNull(), ::testing::IsNull()))
        .WillOnce(
            testing::DoAll(testing::SetArgPointee<0>(response_headers), testing::Return(true)));

    // Call the function under test
    EXPECT_THROW(mock_rdfox_adapter_->createCursor(connection_id, auth_token, valid_query),
                 std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when the location header format is
 * incorrect after creating a cursor.
 */
TEST_F(RDFoxAdapterTest, CreateCursorWrongLocationHeaderFormat) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string valid_query = RandomUtils::generateRandomString(20);
    const std::string target =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors";
    std::map<std::string, std::string> response_headers = {{"Location", "wrong_format"}};

    // Get the raw pointer to the mock_request_builder_
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    ASSERT_TRUE(mock_request_builder_);

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::post))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setContentType("application/sparql-query"))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAuthorization(auth_token))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setBody(valid_query))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::NotNull(), ::testing::IsNull()))
        .WillOnce(
            testing::DoAll(testing::SetArgPointee<0>(response_headers), testing::Return(true)));

    // Call the function under test
    EXPECT_THROW(mock_rdfox_adapter_->createCursor(connection_id, auth_token, valid_query),
                 std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when cursor advance fails.
 */
TEST_F(RDFoxAdapterTest, AdvanceCursorFailure) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const DataQueryAcceptType accept_type = DataQueryAcceptType::SPARQL_JSON;
    const std::pair<std::string, std::string> valid_operations = {"open", "advance"};
    const std::string operation = RandomUtils::generateRandomInt(0, 1) == 0
                                      ? valid_operations.first
                                      : valid_operations.second;
    const int limit = RandomUtils::generateRandomInt(1, 100);
    const std::string target = "/datastores/" + DATASTORE + "/connections/" + connection_id +
                               "/cursors/" + cursor_id + "?operation=" + operation +
                               "&limit=" + std::to_string(limit);

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::patch))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAuthorization(auth_token))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setAcceptType(queryAcceptTypeToString(accept_type)))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::IsNull(), ::testing::NotNull()))
        .WillOnce(testing::DoAll(testing::Return(false)));

    // Call the function under test
    std::string response;
    EXPECT_FALSE(mock_rdfox_adapter_->RDFoxAdapter::advanceCursor(
        connection_id, auth_token, cursor_id, accept_type, operation, limit, &response));
    EXPECT_EQ(response, "");
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when cursor advance fails due to
 * invalid operation value.
 */
TEST_F(RDFoxAdapterTest, AdvanceCursorInvalidOperation) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string auth_token = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const DataQueryAcceptType accept_type = DataQueryAcceptType::SPARQL_JSON;
    const std::string operation = "invalid_operation";

    const std::string target = "/datastores/" + DATASTORE + "/connections/" + connection_id +
                               "/cursors/" + cursor_id + "?operation=" + operation;

    // Ensure that any request to create the datastore is not made.
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder()).Times(0);

    // Call the function under test
    std::string response;
    EXPECT_FALSE(mock_rdfox_adapter_->RDFoxAdapter::advanceCursor(
        connection_id, auth_token, cursor_id, accept_type, operation, std::nullopt, &response));
    EXPECT_EQ(response, "");
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when cursor deletion fails.
 */
TEST_F(RDFoxAdapterTest, DeleteCursorFailure) {
    const std::string connection_id = RandomUtils::generateRandomString(8);
    const std::string cursor_id = RandomUtils::generateRandomString(8);
    const std::string target =
        "/datastores/" + DATASTORE + "/connections/" + connection_id + "/cursors/" + cursor_id;

    // Create a mock RequestBuilder
    MockRequestBuilder* mock_request_builder_ptr = mock_request_builder_.get();

    // Mock createRequestBuilder to return the mock RequestBuilder
    EXPECT_CALL(*mock_rdfox_adapter_, createRequestBuilder())
        .WillOnce(testing::Return(::testing::ByMove(std::move(mock_request_builder_))));

    // Set up expectations for the mock RequestBuilder
    EXPECT_CALL(*mock_request_builder_ptr, setMethod(http::verb::delete_))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, setTarget(target))
        .WillOnce(testing::ReturnRef(*mock_request_builder_ptr));
    EXPECT_CALL(*mock_request_builder_ptr, sendRequest(::testing::IsNull(), ::testing::IsNull()))
        .WillOnce(testing::Return(false));

    // Assertions
    EXPECT_FALSE(mock_rdfox_adapter_->RDFoxAdapter::deleteCursor(connection_id, cursor_id));
}