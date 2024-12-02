#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "rdfox_adapter.h"

class MockRDFoxAdapter : public RDFoxAdapter {
   public:
    MockRDFoxAdapter() : RDFoxAdapter("localhost", "8080", "test_auth", "test_ds") {}

    MOCK_METHOD6(sendRequest, bool(http::verb, const std::string&, const std::string&,
                                   const std::string&, const std::string&, std::string&));
};

/**
 * @brief Unit test for RDFoxAdapter to check the behavior when a datastore does not exist.
 */
TEST(RDFoxAdapterTest, CheckNonexistentDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking nonexistent datastore.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(false)));

    EXPECT_FALSE(mock_adapter.checkDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when a datastore exists.
 */
TEST(RDFoxAdapterTest, CheckExistentDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking existent datastore.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    EXPECT_TRUE(mock_adapter.checkDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the initialization process creates a datastore.
 */
TEST(RDFoxAdapterTest, InitializationCreatesDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking datastore does not exists.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(false)));

    // Mock the sendRequest for creating the datastore.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds", "",
                                          "application/json", "", testing::_))
        .WillOnce(testing::Return(true));

    EXPECT_NO_THROW(mock_adapter.initialize());
}
/**
 * @brief Unit test for RDFoxAdapter to verify the behavior during initialization
 * when a datastore already exists.
 */
TEST(RDFoxAdapterTest, InitializationWithExistingDatastore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking datastore exists.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    // Ensure that the second call for creating the datastore is not made.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds", "",
                                          "application/json", "", testing::_))
        .Times(0);

    EXPECT_NO_THROW(mock_adapter.initialize());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when loading data into a datastore.
 */
TEST(RDFoxAdapterTest, LoadDataSuccess) {
    MockRDFoxAdapter mock_adapter;
    std::string ttl_data = "@prefix : <http://example.org/> . :test a :Entity .";

    // Mock the sendRequest for loading data.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/content", ttl_data,
                                          "text/turtle", "", testing::_))
        .WillOnce(testing::Return(true));

    EXPECT_TRUE(mock_adapter.loadData(ttl_data));
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful data querying.
 */
TEST(RDFoxAdapterTest, QueryDataSuccess) {
    MockRDFoxAdapter mock_adapter;
    std::string sparql_query = "SELECT ?s WHERE { ?s ?p ?o . }";
    std::string mock_response = "<http://example.org/test>";

    // Mock the sendRequest for querying data.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/sparql",
                                          sparql_query, "application/sparql-query", "", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(mock_response), testing::Return(true)));

    EXPECT_EQ(mock_adapter.queryData(sparql_query), mock_response);
}

/**
 * @brief Unit test for RDFoxAdapter to verify successful deletion of a datastore.
 */
TEST(RDFoxAdapterTest, DeleteDataStoreSuccess) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    // Mock the sendRequest for deleting the datastore.
    EXPECT_CALL(mock_adapter,
                sendRequest(http::verb::delete_, "/datastores/test_ds", "", "", "", testing::_))
        .WillOnce(testing::Return(true));

    EXPECT_TRUE(mock_adapter.deleteDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when attempting to delete a nonexistent
 * datastore.
 */
TEST(RDFoxAdapterTest, DeleteNonexistentDataStoreSuccess) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(false)));

    // Ensure that the second call for deleting the datastore is not made.
    EXPECT_CALL(mock_adapter,
                sendRequest(http::verb::delete_, "/datastores/test_ds", "", "", "", testing::_))
        .Times(0);

    EXPECT_TRUE(mock_adapter.deleteDataStore());
}

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when datastore creation fails.
 */
TEST(RDFoxAdapterTest, FailedToCreateDataStore) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>(""), testing::Return(true)));

    // Mock the sendRequest for creating the datastore failure.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds", "",
                                          "application/json", "", testing::_))
        .WillOnce(testing::Return(false));

    // Expect a runtime error when the adapter tries to create the datastore.
    EXPECT_THROW(mock_adapter.initialize(), std::runtime_error);
}

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when loading data fails.
 */
TEST(RDFoxAdapterTest, LoadDataFailure) {
    MockRDFoxAdapter mock_adapter;
    std::string ttl_data = "@prefix : <http://example.org/> . :test a :Entity .";

    // Mock the sendRequest for loading data failure.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/content", ttl_data,
                                          "text/turtle", "", testing::_))
        .WillOnce(testing::Return(false));

    EXPECT_FALSE(mock_adapter.loadData(ttl_data));
}

/**
 * @brief Unit test for RDFoxAdapter to verify behavior when a SPARQL query fails.
 */
TEST(RDFoxAdapterTest, QueryDataFailure) {
    MockRDFoxAdapter mock_adapter;
    std::string sparql_query = "SELECT ?s WHERE { ?s ?p ?o . }";

    // Mock the sendRequest for querying data failure.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::post, "/datastores/test_ds/sparql",
                                          sparql_query, "application/sparql-query", "", testing::_))
        .WillOnce(testing::Return(false));

    // Call queryData and expect it to return an empty string.
    EXPECT_EQ(mock_adapter.queryData(sparql_query), "");
}

/**
 * @brief Unit test for RDFoxAdapter to verify the behavior when attempting to delete a datastore
 * fails.
 */
TEST(RDFoxAdapterTest, DeleteDataStoreFailure) {
    MockRDFoxAdapter mock_adapter;

    // Mock the sendRequest for checking the datastore existence.
    EXPECT_CALL(mock_adapter, sendRequest(http::verb::get, "/datastores", "", "",
                                          "text/csv; charset=UTF-8", testing::_))
        .WillOnce(testing::DoAll(testing::SetArgReferee<5>("test_ds"), testing::Return(true)));

    // Mock the sendRequest for deleting the datastore failure.
    EXPECT_CALL(mock_adapter,
                sendRequest(http::verb::delete_, "/datastores/test_ds", "", "", "", testing::_))
        .WillOnce(testing::Return(false));

    EXPECT_FALSE(mock_adapter.deleteDataStore());
}