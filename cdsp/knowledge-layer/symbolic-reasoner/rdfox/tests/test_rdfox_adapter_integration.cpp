#include <gtest/gtest.h>

#include <string>

#include "../src/rdfox-adapter.h"

class RDFoxAdapterIntegrationTest : public ::testing::Test {
   protected:
    RDFoxAdapter* adapter;

    std::string host = "localhost";
    std::string port = "12110";
    std::string auth_base64 = "cm9vdDphZG1pbg==";  // Base64 encoded authorization string
    std::string data_store = "test_ds";

    void SetUp() override {
        adapter = new RDFoxAdapter(host, port, auth_base64, data_store);
        adapter->initialize();

        // Initial load to ensure datastore is accessible
        if (!adapter->checkDataStore()) {
            FAIL() << "Failed to ensure the data store is set up.";
        }
    }

    void TearDown() override {
        // Delete the test data store after all tests have completed
        // ASSERT_TRUE(adapter->deleteDataStore()) << "Failed to clean up the test data store.";
        adapter->deleteDataStore();

        // Clean up the adapter instance
        delete adapter;
    }
};

// Integration Test: Ensure RDFoxAdapter can create a data store and retrieve it
TEST_F(RDFoxAdapterIntegrationTest, DataStoreInitializationTest) {
    // This test is implicitly done during the setup phase of RDFoxAdapter
    // If the data store exists or is created, no error will be thrown
    // You can further assert behavior here if needed
    SUCCEED();
}

TEST_F(RDFoxAdapterIntegrationTest, LoadDataTest) {
    std::string ttlData = R"(
        @prefix bmw: <http://example.com/bmw#> .
        bmw:Vehicle1 a bmw:Vehicle .
    )";

    // Try loading data into the data store
    ASSERT_TRUE(adapter->loadData(ttlData)) << "Failed to load Turtle data into the store";
}

TEST_F(RDFoxAdapterIntegrationTest, QueryDataTest) {
    std::string ttlData = R"(
        @prefix bmw: <http://example.com/bmw#> .
        bmw:Vehicle1 a bmw:Vehicle .
    )";
    ASSERT_TRUE(adapter->loadData(ttlData)) << "Failed to load Turtle data into the store";

    std::string sparqlQuery = "SELECT ?s ?p ?o WHERE { ?s ?p ?o }";
    std::string result = adapter->queryData(sparqlQuery);

    ASSERT_FALSE(result.empty()) << "SPARQL query returned no results";
    EXPECT_TRUE(result.find("<http://example.com/bmw#Vehicle1>") != std::string::npos)
        << "Result does not contain the expected subject.";
    EXPECT_TRUE(result.find("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>") !=
                std::string::npos)
        << "Result does not contain the expected predicate.";
    EXPECT_TRUE(result.find("<http://example.com/bmw#Vehicle>") != std::string::npos)
        << "Result does not contain the expected object.";
}

TEST_F(RDFoxAdapterIntegrationTest, DeleteDataStoreTest) {
    ASSERT_TRUE(adapter->checkDataStore()) << "Any data store has been created.";
    ASSERT_TRUE(adapter->deleteDataStore()) << "Failed to delete the data store.";
    ASSERT_FALSE(adapter->checkDataStore()) << "The datastore might still exist or be recreated.";
}