#include <gtest/gtest.h>

#include "data_types.h"
#include "reasoner_factory.h"
#include "reasoner_service.h"
#include "server_data_fixture.h"

class ReasonerFactoryIntegrationTest : public ::testing::Test {
   protected:
    const ReasonerServerData server_data_ = ServerDataFixture::getValidRDFoxServerData();
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

    // Arrange Ontology definition
    std::string ontology_data = R"(
        @prefix ex: <http://example.org/> .
        @prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
        @prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .

        # Define the Animal class
        ex:Animal a rdfs:Class .

        # Define the hasLegs property
        ex:hasLegs a rdf:Property ;
            rdfs:domain ex:Animal ;
            rdfs:range rdfs:Literal .)";

    // Arrange Rule definition
    std::string rule_data = R"(
        @prefix ex: <http://example.org/> .
        @prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

        # If an entity has legs (> 0), it is an Animal with Legs
        [ ?entity, ex:isLeggedAnimal, "true" ] :- 
            [ ?entity, ex:hasLegs, ?legs ], 
            FILTER(?legs > 0) .

        # If an entity has no legs (= 0), it is a Legless Animal
        [ ?entity, ex:isLeglessAnimal, "true" ] :- 
            [ ?entity, ex:hasLegs, "0"^^xsd:integer ] .)";

    const std::vector<std::pair<RuleLanguageType, std::string>> reasoner_rules = {
        {RuleLanguageType::DATALOG, rule_data}};

    const std::vector<std::pair<ReasonerSyntaxType, std::string>> ontologies = {
        {ReasonerSyntaxType::TURTLE, ontology_data}};

    // Act
    reasoner_service_ = factory.initReasoner(InferenceEngineType::RDFOX, server_data_,
                                             reasoner_rules, ontologies, false);

    // Assert reasoner adapter has been initialized
    EXPECT_TRUE(reasoner_service_->checkDataStore());

    // We test the rule by querying the data
    std::string ttl_data = R"(
        @prefix ex: <http://example.org/> .
        ex:Dog ex:hasLegs "4"^^xsd:integer .
        ex:Bird ex:hasLegs "2"^^xsd:integer .
        ex:Snake ex:hasLegs "0"^^xsd:integer .)";

    reasoner_service_->loadData(ttl_data, ReasonerSyntaxType::TURTLE);

    // Query to get all legged animals
    std::string sparql_query_legged_animals = R"(
        PREFIX ex: <http://example.org/>
        SELECT ?entity WHERE { ?entity ex:isLeggedAnimal "true" . }
    )";

    std::string expected_result_legged_animals =
        "?entity\n<http://example.org/Dog>\n<http://example.org/Bird>\n";

    std::string query_result = reasoner_service_->queryData(
        sparql_query_legged_animals, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_TSV);

    EXPECT_EQ(query_result, expected_result_legged_animals);

    // Query to get all legless animals
    std::string sparql_query_legless_animals = R"(
        PREFIX ex: <http://example.org/>
        SELECT ?entity WHERE { ?entity ex:isLeglessAnimal "true" . }
    )";

    std::string expected_result_legless_animals = "?entity\n<http://example.org/Snake>\n";

    std::string query_result_legless_animals = reasoner_service_->queryData(
        sparql_query_legless_animals, QueryLanguageType::SPARQL, DataQueryAcceptType::TEXT_TSV);

    EXPECT_EQ(query_result_legless_animals, expected_result_legless_animals);
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
    const std::vector<std::pair<ReasonerSyntaxType, std::string>> ontologies = {};

    // Act & Assert
    EXPECT_THROW(
        reasoner_service_ = factory.initReasoner(static_cast<InferenceEngineType>(-1), server_data_,
                                                 reasoner_rules, ontologies, false),
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
    const std::vector<std::pair<ReasonerSyntaxType, std::string>> ontologies = {};

    ReasonerServerData invalid_server_data = {"invalid_host", "invalid_port", "invalid_auth",
                                              "some_datastore"};

    // Act & Assert
    EXPECT_THROW(
        reasoner_service_ = factory.initReasoner(InferenceEngineType::RDFOX, invalid_server_data,
                                                 reasoner_rules, ontologies, false),
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
    const std::vector<std::pair<ReasonerSyntaxType, std::string>> ontologies = {};

    // Act & Assert
    EXPECT_THROW(
        reasoner_service_ = factory.initReasoner(InferenceEngineType::RDFOX, server_data_,
                                                 invalid_reasoner_rules, ontologies, false),
        std::runtime_error);
}

/**
 * @brief Test to verify that an exception is thrown when invalid ontologies are provided.
 *
 * This test checks that the ReasonerFactory throws a std::runtime_error exception
 * when attempting to initialize a Reasoner with invalid ontologies.
 * It ensures that the factory correctly handles invalid input by preventing
 * the creation of a reasoner service.
 */
TEST_F(ReasonerFactoryIntegrationTest, InitializeReasonerWithInvalidOntologiesFailure) {
    // Arrange
    ReasonerFactory factory;
    const std::vector<std::pair<RuleLanguageType, std::string>> reasoner_rules = {};
    const std::vector<std::pair<ReasonerSyntaxType, std::string>> invalid_ontologies = {
        {ReasonerSyntaxType::TURTLE, "invalid_ontology"}};

    // Act & Assert
    EXPECT_THROW(
        reasoner_service_ = factory.initReasoner(InferenceEngineType::RDFOX, server_data_,
                                                 reasoner_rules, invalid_ontologies, false),
        std::runtime_error);
}