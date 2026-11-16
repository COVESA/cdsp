#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "helper.h"
#include "mock_cqels_adapter.h"
#include "request_expectations.h"

class CQELSAdapterTest : public ::testing::Test {
    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
   protected:
    const std::string DATASTORE = "ds-test";
    const std::string QUERY = R"(REGISTER QUERY Speed AS
SELECT ?car ?speed
FROM STREAM Signals [RANGE 5s]
FROM STREAM Angles [RANGE 5s]
WHERE { STREAM Signals { ?car <http://example.org/speed> ?speed } })";
    const std::string OBSERVATION = "<s> <p> <o> .";
    const ReasonerServerData server_data_{"localhost", "8080", "auth", "", DATASTORE};

    std::shared_ptr<MockCQELSAdapter> adapter_;

    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)
    void SetUp() override { adapter_ = std::make_shared<MockCQELSAdapter>(server_data_); }

    bool registerQuery() {
        return adapter_->loadRules(QUERY, ruleLanguageTypeToContentType(RuleLanguageType::CQELS));
    }

    ExpectedRequest registerQueryRequest(const std::string& query, const std::string& query_id) {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/queries")
            .withContentType("text/plain")
            .withBody(query)
            .respondsWith(query_id);
    }

    ExpectedRequest pushRequest(const std::string& stream, const std::string& event_time) {
        return ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/streams/" + stream +
                                                     "?eventTime=" + event_time)
            .withContentType("text/turtle")
            .withBody(OBSERVATION);
    }

    ExpectedRequest listDatastoresRequest(const std::string& datastores) {
        return ExpectedRequest(http::verb::get, "/datastores")
            .withAcceptType("text/plain")
            .respondsWith(datastores);
    }
};

/**
 * @brief A registered query is fetched by its id when the same query text is queried.
 */
TEST_F(CQELSAdapterTest, RegisteredQueryReturnsItsResults) {
    const std::string results = R"({"results":{"bindings":[]}})";
    expectRequests(*adapter_, {registerQueryRequest(QUERY, "Speed"),
                               ExpectedRequest(http::verb::get, "/datastores/" + DATASTORE +
                                                                    "/queries/Speed/results")
                                   .withAcceptType("text/tab-separated-values")
                                   .respondsWith(results)});

    ASSERT_TRUE(registerQuery());
    EXPECT_EQ(adapter_->queryData(QUERY, QueryLanguageType::CQELS), results);
}

/**
 * @brief Observations go to every stream of the registered queries, with the observation time as
 * event time.
 */
TEST_F(CQELSAdapterTest, ObservationsArePushedToEveryStreamWithTheEventTime) {
    const auto observation_time =
        Helper::convertToTimestamp(1793355330, 250000000);  // 2026-10-30T10:15:30.25Z
    const std::string event_time = "2026-10-30T10:15:30.250000000Z";
    expectRequests(*adapter_,
                   {registerQueryRequest(QUERY, "Speed"), pushRequest("Angles", event_time),
                    pushRequest("Signals", event_time)});

    ASSERT_TRUE(registerQuery());
    EXPECT_TRUE(adapter_->loadObservations(OBSERVATION, "text/turtle", observation_time));
}

/**
 * @brief Stream names may contain hyphens, as in the CQELS-QL grammar.
 */
TEST_F(CQELSAdapterTest, StreamNamesWithHyphensAreKept) {
    const std::string query =
        "REGISTER QUERY Q AS SELECT ?s FROM STREAM vehicle-signals [RANGE 5s] "
        "WHERE { STREAM vehicle-signals { ?s ?p ?o } }";
    const auto observation_time = Helper::convertToTimestamp(1793355330, 0);
    expectRequests(*adapter_, {registerQueryRequest(query, "Q"),
                               pushRequest("vehicle-signals", "2026-10-30T10:15:30.000000000Z")});

    ASSERT_TRUE(adapter_->loadRules(query, ruleLanguageTypeToContentType(RuleLanguageType::CQELS)));
    EXPECT_TRUE(adapter_->loadObservations(OBSERVATION, "text/turtle", observation_time));
}

/**
 * @brief Observations without a registered query have no stream to go to.
 */
TEST_F(CQELSAdapterTest, ObservationsWithoutRegisteredQueryFail) {
    expectRequests(*adapter_, {});

    EXPECT_FALSE(
        adapter_->loadObservations(OBSERVATION, "text/turtle", std::chrono::system_clock::now()));
}

/**
 * @brief Rules in other languages than CQELS-QL are rejected without a request.
 */
TEST_F(CQELSAdapterTest, NonCqelsRulesAreRejected) {
    expectRequests(*adapter_, {});

    EXPECT_FALSE(adapter_->loadRules("@prefix sh: <http://www.w3.org/ns/shacl#> .",
                                     ruleLanguageTypeToContentType(RuleLanguageType::SHACL)));
}

/**
 * @brief A CQELS-QL query that was not registered returns an empty result without a request.
 */
TEST_F(CQELSAdapterTest, UnregisteredQueryReturnsEmpty) {
    expectRequests(*adapter_, {});

    EXPECT_EQ(adapter_->queryData(QUERY, QueryLanguageType::CQELS), "");
}

/**
 * @brief SPARQL queries run on the static data.
 */
TEST_F(CQELSAdapterTest, SparqlQueryRunsOnStaticData) {
    const std::string query = "SELECT * WHERE { ?s ?p ?o }";
    expectRequests(*adapter_,
                   {ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/sparql")
                        .withContentType("application/sparql-query")
                        .withBody(query)
                        .withAcceptType("text/tab-separated-values")
                        .respondsWith("result")});

    EXPECT_EQ(adapter_->queryData(query, QueryLanguageType::SPARQL), "result");
}

/**
 * @brief Initialization creates the datastore when the service does not list it.
 */
TEST_F(CQELSAdapterTest, InitializationCreatesMissingDatastore) {
    expectRequests(*adapter_, {listDatastoresRequest("other-ds"),
                               ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE)});

    EXPECT_NO_THROW(adapter_->initialize());
}

/**
 * @brief Initialization creates the datastore when the service only lists one with a longer name.
 */
TEST_F(CQELSAdapterTest, InitializationMatchesTheExactDatastoreName) {
    expectRequests(*adapter_, {listDatastoresRequest("other, " + DATASTORE + "-2"),
                               ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE)});

    EXPECT_NO_THROW(adapter_->initialize());
}

/**
 * @brief Static data is loaded into the datastore content.
 */
TEST_F(CQELSAdapterTest, StaticDataIsLoaded) {
    expectRequests(*adapter_,
                   {ExpectedRequest(http::verb::post, "/datastores/" + DATASTORE + "/content")
                        .withContentType("text/turtle")
                        .withBody(OBSERVATION)});

    EXPECT_TRUE(adapter_->loadData(OBSERVATION, "text/turtle"));
}

/**
 * @brief An existing datastore is deleted.
 */
TEST_F(CQELSAdapterTest, ExistingDatastoreIsDeleted) {
    expectRequests(*adapter_, {listDatastoresRequest(DATASTORE),
                               ExpectedRequest(http::verb::delete_, "/datastores/" + DATASTORE)});

    EXPECT_TRUE(adapter_->deleteDataStore());
}
