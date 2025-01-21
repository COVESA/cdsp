#include <gtest/gtest.h>

#include <string>

#include "data_types.h"
#include "random_utils.h"
#include "rdfox_adapter.h"
#include "server_data_fixture.h"

class RDFoxAdapterIntegrationTest : public ::testing::Test {
   protected:
    RDFoxAdapter* adapter;

    const ServerData rdfox_server_ = ServerDataFixture::getValidRDFoxServerData();
    const std::string triple_subject_ = RandomUtils::generateRandomString(8);
    const std::string triple_object_ = RandomUtils::generateRandomString(6);

    void SetUp() override {
        adapter = new RDFoxAdapter(rdfox_server_.host, rdfox_server_.port,
                                   rdfox_server_.auth_base64, rdfox_server_.data_store.value());
        adapter->initialize();

        // Initial load to ensure datastore is accessible
        if (!adapter->checkDataStore()) {
            FAIL() << "Failed to ensure the data store is set up.";
        }
    }

    void TearDown() override {
        // Delete the test data store after all tests have completed
        ASSERT_TRUE(adapter->deleteDataStore()) << "Failed to clean up the test data store.";

        // Clean up the adapter instance
        delete adapter;
    }
};

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
 * @brief Test to verify the loading of Turtle data into the RDFox data store.
 *
 * This test creates a simple Turtle data string and attempts to load it into
 * the RDFox data store using the RDFoxAdapter. The test asserts that the data
 * is loaded successfully. If the loading fails, an error message is displayed.
 */
TEST_F(RDFoxAdapterIntegrationTest, LoadDataTest) {
    // Define a Turtle data string with a namespace prefix and a single triple
    std::string ttlData = R"(
        @prefix car: <http://example.com/car#> .
        car:)" + triple_subject_ +
                          R"( a car:)" + triple_object_ + R"( .
    )";

    // Attempt to load the Turtle data into the data store and assert success
    ASSERT_TRUE(adapter->loadData(ttlData)) << "Failed to load Turtle data into the store";
}

/**
 * @brief Test to verify querying data from the RDFox data store.
 *
 * This test loads a simple Turtle data string into the RDFox data store and
 * performs a SPARQL query to retrieve all triples. The test asserts that the
 * data is loaded successfully and that the query returns the expected results.
 * Specifically, it checks for the presence of a subject, predicate, and object
 * in the query results.
 */
TEST_F(RDFoxAdapterIntegrationTest, QueryDataTest) {
    // Define a Turtle data string with a namespace prefix and a single triple
    std::string ttlData = R"(
        @prefix car: <http://example.com/car#> .
        car:)" + triple_subject_ +
                          R"( a car:)" + triple_object_ + R"( .
    )";

    // Attempt to load the Turtle data into the data store and assert success
    ASSERT_TRUE(adapter->loadData(ttlData)) << "Failed to load Turtle data into the store";

    // Define a SPARQL query to select all triples in the data store
    std::string sparqlQuery = "SELECT ?s ?p ?o WHERE { ?s ?p ?o }";

    // Execute the SPARQL query and store the result
    std::string result = adapter->queryData(sparqlQuery);

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
    ASSERT_TRUE(adapter->checkDataStore()) << "Any data store has been created.";
    ASSERT_TRUE(adapter->deleteDataStore()) << "Failed to delete the data store.";
    ASSERT_FALSE(adapter->checkDataStore()) << "The datastore might still exist or be recreated.";
}
