#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>

#include "cqels_adapter.h"
#include "data_types.h"
#include "random_utils.h"
#include "server_data_fixture.h"

class CQELSAdapterIntegrationTest : public ::testing::Test {
   protected:
    CQELSAdapter* adapter_;

    const ReasonerServerData cqels_server_ = ServerDataFixture::getValidCQELSServerData();
    const std::string triple_subject_ = RandomUtils::generateRandomString(8);
    const std::string triple_object_ = RandomUtils::generateRandomString(6);

    void SetUp() override {
        adapter_ = new CQELSAdapter(cqels_server_);
        adapter_->initialize();

        if (!adapter_->checkDataStore()) {
            FAIL() << "Failed to ensure the data store is set up.";
        }
    }

    void TearDown() override {
        ASSERT_TRUE(adapter_->deleteDataStore()) << "Failed to clean up the test data store.";
        delete adapter_;
    }
};

/**
 * @brief Test to verify implicit initialization of the data store via SetUp.
 */
TEST_F(CQELSAdapterIntegrationTest, DataStoreInitializationTest) { SUCCEED(); }

/**
 * @brief Test to verify loading static data and querying it back with SPARQL.
 */
TEST_F(CQELSAdapterIntegrationTest, LoadAndQueryStaticData) {
    const std::string turtle_data = R"(
        @prefix car: <http://example.com/car#> .
        car:)" + triple_subject_ + R"( a car:)" +
                                    triple_object_ + R"( .
    )";

    ASSERT_TRUE(adapter_->loadData(turtle_data,
                                   reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE)))
        << "Failed to load Turtle data.";

    const std::string result = adapter_->queryData("SELECT ?s ?p ?o WHERE { ?s ?p ?o }");

    EXPECT_NE(result.find("<http://example.com/car#" + triple_subject_ + ">"), std::string::npos)
        << "Result does not contain the expected subject.";
    EXPECT_NE(result.find("<http://example.com/car#" + triple_object_ + ">"), std::string::npos)
        << "Result does not contain the expected object.";
}

/**
 * @brief Test to verify that pushed observations give results of a registered query, and that
 * each result is returned only once.
 *
 * The service holds elements in its reorder buffer for a short time, so the results are polled.
 */
TEST_F(CQELSAdapterIntegrationTest, RegisteredQueryReturnsResultsOfPushedObservations) {
    const std::string query = R"(
        PREFIX car: <http://example.com/car#>
        REGISTER QUERY Speed AS
        SELECT ?car ?speed
        FROM STREAM Signals [RANGE 5s]
        WHERE { STREAM Signals { ?car car:speed ?speed } }
    )";
    const std::string observation =
        "@prefix car: <http://example.com/car#> . car:" + triple_subject_ + " car:speed \"" +
        triple_object_ + "\" .";

    ASSERT_TRUE(adapter_->loadRules(query, ruleLanguageTypeToContentType(RuleLanguageType::CQELS)))
        << "Failed to register the CQELS-QL query.";
    ASSERT_TRUE(adapter_->loadObservations(
        observation, reasonerSyntaxTypeToContentType(ReasonerSyntaxType::TURTLE),
        std::chrono::system_clock::now()))
        << "Failed to push the observation.";

    std::string result;
    for (int attempt = 0; attempt < 30 && result.find(triple_object_) == std::string::npos;
         attempt++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        result =
            adapter_->queryData(query, QueryLanguageType::CQELS, DataQueryAcceptType::SPARQL_JSON);
    }

    EXPECT_NE(result.find(triple_object_), std::string::npos)
        << "Results do not contain the pushed observation: " << result;

    const std::string next_result =
        adapter_->queryData(query, QueryLanguageType::CQELS, DataQueryAcceptType::SPARQL_JSON);
    EXPECT_EQ(next_result.find(triple_object_), std::string::npos)
        << "A result was returned twice: " << next_result;
}
