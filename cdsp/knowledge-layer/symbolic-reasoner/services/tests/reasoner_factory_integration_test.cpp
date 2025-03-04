#include <gtest/gtest.h>

#include "data_types.h"
#include "reasoner_factory.h"
#include "reasoner_service.h"
#include "server_data_fixture.h"

class ReasonerFactoryIntegrationTest : public ::testing::Test {
   protected:
    const ServerData server_data_ = ServerDataFixture::getValidRDFoxServerData();
    std::shared_ptr<ReasonerService> reasoner_service_;

    void SetUp() override {}

    void TearDown() override {
        // Clean up
        if (reasoner_service_) {
            reasoner_service_->deleteDataStore();
        }
    }
};

/**
 * @brief Test to verify the successful initialization of a Reasoner with RDFox adapter.
 *
 * This test checks the initialization of a Reasoner using the RDFox adapter and ensures
 * that the reasoner is correctly set up with the provided rules.
 */
TEST_F(ReasonerFactoryIntegrationTest, InitializeReasonerWithRDFoxAdapterSuccess) {
    // Arrange
    ReasonerFactory factory;
    std::string rule_data = R"(
        @prefix ex: <http://example.org/> .

        [ ?entity, ex:isPerson, "true" ] :- [ ?entity, ex:hasName, ?name ] .)";

    const std::vector<std::pair<RuleLanguageType, std::string>> reasoner_rules = {
        {RuleLanguageType::DATALOG, rule_data}};

    // Act
    reasoner_service_ =
        factory.initReasoner(InferenceEngineType::RDFOX, server_data_, reasoner_rules);

    // Assert reasoner adapter has been initialized
    EXPECT_TRUE(reasoner_service_->checkDataStore());

    // We test the rule by querying the data
    std::string ttl_data = R"(
        @prefix ex: <http://example.org/> .
        ex:entity_john ex:hasName "John Doe" .)";

    reasoner_service_->loadData(ttl_data, ReasonerSyntaxType::TURTLE);

    std::string sparql_query = R"(
        PREFIX ex: <http://example.org/>
        SELECT ?entity WHERE { ?entity ex:isPerson "true" . }
    )";

    std::string expected_result = "?entity\n<http://example.org/entity_john>\n";

    std::string query_result = reasoner_service_->queryData(sparql_query, QueryLanguageType::SPARQL,
                                                            DataQueryAcceptType::TEXT_TSV);

    EXPECT_EQ(query_result, expected_result);
}

/**
 * @brief Test to verify that an exception is thrown when an invalid inference engine is used.
 *
 * This test checks that the ReasonerFactory throws a std::invalid_argument exception
 * when attempting to initialize a Reasoner with an invalid InferenceEngineType.
 * It ensures that the factory correctly handles invalid input by preventing
 * the creation of a reasoner service.
 */
TEST_F(ReasonerFactoryIntegrationTest, ThrowExceptionWhenInferenceEngineIsInvalid) {
    // Arrange
    ReasonerFactory factory;
    const std::vector<std::pair<RuleLanguageType, std::string>> reasoner_rules = {};

    // Act & Assert
    EXPECT_THROW(reasoner_service_ = factory.initReasoner(static_cast<InferenceEngineType>(-1),
                                                          server_data_, reasoner_rules),
                 std::invalid_argument);
}

/**
 * @brief Test to verify that an exception is thrown when invalid server data is used.
 *
 * This test checks that the ReasonerFactory throws a std::runtime_error exception
 * when attempting to initialize a Reasoner with invalid server data.
 * It ensures that the factory correctly handles invalid input by preventing
 * the creation of a reasoner service.
 */
TEST_F(ReasonerFactoryIntegrationTest, InitializeReasonerWithInvalidServerDataFailure) {
    // Arrange
    ReasonerFactory factory;
    const std::vector<std::pair<RuleLanguageType, std::string>> reasoner_rules = {};

    ServerData invalid_server_data = {"invalid_host", "invalid_port", "invalid_auth",
                                      "some_datastore"};

    // Act & Assert
    EXPECT_THROW(reasoner_service_ = factory.initReasoner(InferenceEngineType::RDFOX,
                                                          invalid_server_data, reasoner_rules),
                 std::runtime_error);
}

/**
 * @brief Test to verify that an exception is thrown when invalid rules are provided.
 *
 * This test checks that the ReasonerFactory throws a std::runtime_error exception
 * when attempting to initialize a Reasoner with invalid rules.
 * It ensures that the factory correctly handles invalid input by preventing
 * the creation of a reasoner service.
 */
TEST_F(ReasonerFactoryIntegrationTest, InitializeReasonerWithInvalidRulesFailure) {
    // Arrange
    ReasonerFactory factory;
    const std::vector<std::pair<RuleLanguageType, std::string>> invalid_reasoner_rules = {
        {RuleLanguageType::DATALOG, "invalid_rule"}};

    // Act & Assert
    EXPECT_THROW(reasoner_service_ = factory.initReasoner(InferenceEngineType::RDFOX, server_data_,
                                                          invalid_reasoner_rules),
                 std::runtime_error);
}
