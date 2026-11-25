#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "mock_rdf4j_adapter.h"
#include "request_expectations.h"

class RDF4JAdapterTest : public ::testing::Test {
    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
   protected:
    const std::string DATASTORE = "ds-test";
    const std::string TTL_DATA = "@prefix : <http://example.org/> . :test a :Entity .";
    const std::string SHACL_RULES =
        "@prefix sh: <http://www.w3.org/ns/shacl#> . :MyShape a sh:NodeShape .";
    const std::string SPARQL_QUERY = "SELECT ?s WHERE { ?s ?p ?o . }";
    const ReasonerServerData server_data_{"localhost", "8080", "auth", "", DATASTORE};

    std::shared_ptr<MockRDF4JAdapter> adapter_;

    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)
    void SetUp() override { adapter_ = std::make_shared<MockRDF4JAdapter>(server_data_); }

    ExpectedRequest listDatastoresRequest(const std::string& datastores) {
        return ExpectedRequest(http::verb::get, "/datastores")
            .withAcceptType("text/plain")
            .respondsWith(datastores);
    }

    ExpectedRequest loadDataRequest() {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/content")
            .withContentType(reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE))
            .withBody(TTL_DATA);
    }

    ExpectedRequest loadRulesRequest() {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/rules/add")
            .withContentType("text/turtle")
            .withBody(SHACL_RULES);
    }

    ExpectedRequest queryRequest() {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/sparql")
            .withContentType("application/sparql-query")
            .withBody(SPARQL_QUERY)
            .withAcceptType(queryAcceptTypeToString(DataQueryAcceptType::TEXT_TSV));
    }
};

// Unit tests for RDF4JAdapter to verify success regular operations

/**
 * @brief Test fixture for RDF4JAdapter to check the existence of a datastore.
 */
TEST_F(RDF4JAdapterTest, CheckExistentDatastore) {
    expectRequests(*adapter_, {listDatastoresRequest(DATASTORE)});

    EXPECT_TRUE(adapter_->checkDataStore());
}

/**
 * @brief A datastore whose name only starts with the configured name does not count.
 */
TEST_F(RDF4JAdapterTest, CheckDatastoreMatchesTheExactName) {
    expectRequests(*adapter_, {listDatastoresRequest("other, ds-test-2")});

    EXPECT_FALSE(adapter_->checkDataStore());
}

/**
 * @brief Unit test for RDF4JAdapter to verify the initialization process creates a datastore.
 */
TEST_F(RDF4JAdapterTest, InitializationCreatesDatastore) {
    expectRequests(*adapter_, {listDatastoresRequest("other"),
                               ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE)});

    EXPECT_NO_THROW(adapter_->initialize());
}

/**
 * @brief Unit test for RDF4JAdapter to verify initialization skips creation when datastore exists.
 */
TEST_F(RDF4JAdapterTest, InitializationWithExistingDatastore) {
    expectRequests(*adapter_, {listDatastoresRequest(DATASTORE)});

    EXPECT_NO_THROW(adapter_->initialize());
}

/**
 * @brief Unit test for RDF4JAdapter to verify successful data loading.
 */
TEST_F(RDF4JAdapterTest, LoadDataSuccess) {
    expectRequests(*adapter_, {loadDataRequest()});

    EXPECT_TRUE(
        adapter_->loadData(TTL_DATA, reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE)));
}

/**
 * @brief Unit test for RDF4JAdapter to verify that loadRules posts to the rules/add endpoint.
 */
TEST_F(RDF4JAdapterTest, LoadRulesSuccess) {
    expectRequests(*adapter_, {loadRulesRequest()});

    EXPECT_TRUE(adapter_->loadRules(SHACL_RULES, "text/turtle"));
}

/**
 * @brief Unit test for RDF4JAdapter to verify successful data querying.
 */
TEST_F(RDF4JAdapterTest, QueryDataSuccess) {
    const std::string mock_response = "<http://example.org/test>";
    expectRequests(*adapter_, {queryRequest().respondsWith(mock_response)});

    EXPECT_EQ(
        adapter_->queryData(SPARQL_QUERY, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_TSV),
        mock_response);
}

/**
 * @brief Unit test for RDF4JAdapter to verify successful deletion of a datastore.
 */
TEST_F(RDF4JAdapterTest, DeleteDataStoreWhenExists) {
    expectRequests(*adapter_, {listDatastoresRequest(DATASTORE),
                               ExpectedRequest(http::verb::delete_, "/datastores/" + DATASTORE)});

    EXPECT_TRUE(adapter_->deleteDataStore());
}

/**
 * @brief Unit test for RDF4JAdapter to verify that deleteDataStore returns true
 * when the datastore does not exist (nothing to delete).
 */
TEST_F(RDF4JAdapterTest, DeleteDataStoreWhenNotExists) {
    expectRequests(*adapter_, {listDatastoresRequest("other")});

    EXPECT_TRUE(adapter_->deleteDataStore());
}

/**
 * @brief Unit test for RDF4JAdapter to verify that deleteDataStore returns false
 * when the DELETE request fails.
 */
TEST_F(RDF4JAdapterTest, DeleteDataStoreWhenExistsFails) {
    expectRequests(*adapter_,
                   {listDatastoresRequest(DATASTORE),
                    ExpectedRequest(http::verb::delete_, "/datastores/" + DATASTORE).fails()});

    EXPECT_FALSE(adapter_->deleteDataStore());
}

// Unit tests for RDF4JAdapter to verify error handling

/**
 * @brief Unit test for RDF4JAdapter to verify behavior when datastore creation fails.
 */
TEST_F(RDF4JAdapterTest, FailedToCreateDataStore) {
    expectRequests(*adapter_,
                   {listDatastoresRequest("other"),
                    ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE).fails()});

    EXPECT_THROW(adapter_->initialize(), std::runtime_error);
}

/**
 * @brief Unit test for RDF4JAdapter to verify behavior when loading data fails.
 */
TEST_F(RDF4JAdapterTest, LoadDataFailure) {
    expectRequests(*adapter_, {loadDataRequest().fails()});

    EXPECT_FALSE(
        adapter_->loadData(TTL_DATA, reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE)));
}

/**
 * @brief Unit test for RDF4JAdapter to verify that loadRules failure is propagated.
 */
TEST_F(RDF4JAdapterTest, LoadRulesFailure) {
    expectRequests(*adapter_, {loadRulesRequest().fails()});

    EXPECT_FALSE(adapter_->loadRules(SHACL_RULES, "text/turtle"));
}

/**
 * @brief Unit test for RDF4JAdapter to verify behavior when a SPARQL query fails.
 */
TEST_F(RDF4JAdapterTest, QueryDataFailure) {
    expectRequests(*adapter_, {queryRequest().respondsWith("").fails()});

    EXPECT_EQ(
        adapter_->queryData(SPARQL_QUERY, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_TSV),
        "");
}
