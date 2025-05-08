#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

#include "observation_id_utils.h"
#include "random_utils.h"
#include "triple_writer.h"
#include "utc_date_utils.h"
#include "vin_utils.h"

class TripleWriterIntegrationTest : public ::testing::Test {
   protected:
    TripleWriter* triple_writer;
    std::string prefixes_fixture_;
    const std::string VIN = VinUtils::getRandomVinString();
    const std::string OBSERVATION_VALUE = std::to_string(RandomUtils::generateRandomFloat(0, 300));
    const std::chrono::system_clock::time_point TIMESTAMP =
        RandomUtils::generateRandomTimestamp(2000, 2030, true);
    const std::string DATA_TIME = UtcDateUtils::formatCustomTimestampAsIso8601(TIMESTAMP);

    const std::string OBSERVATION_IDENTIFIER =
        "ob_CurrentEnergy_" + ObservationIdentifier::createObservationIdentifier(TIMESTAMP);

    void SetUp() override {
        triple_writer = new TripleWriter();
        prefixes_fixture_ =
            R"(prefix car: <http://example.ontology.com/car#>
            prefix middleware: <http://target-nameospace-for-data-middleware#>
            prefix sh: <http://www.w3.org/ns/shacl#>
            prefix val: <http://example.ontology.com/validation#>
            prefix xsd: <http://www.w3.org/2001/XMLSchema#>)";
    }

    void TearDown() override { delete triple_writer; }

    void SetUpRDFObjects(const std::string& prefixes_input) {
        // RDF Object Triples
        std::vector<std::tuple<std::string, std::string, std::string>> object_triples = {
            {"<http://example.ontology.com/car#Vehicle>",
             "<http://example.ontology.com/car#hasPart>",
             "<http://example.ontology.com/car#Powertrain>"},

            {"<http://example.ontology.com/car#Powertrain>",
             "<http://example.ontology.com/car#hasPart>",
             "<http://example.ontology.com/car#TractionBattery>"},

            {"<http://example.ontology.com/car#TractionBattery>",
             "<http://example.ontology.com/car#hasSignal>",
             "<http://example.ontology.com/car#StateOfCharge>"}};

        for (const auto& triple : object_triples) {
            triple_writer->addElementObjectToTriple(prefixes_input, triple);
        }
    }

    void SetUpRDFData(const std::string& prefixes_input) {
        // RDF Data Triple
        triple_writer->addElementDataToTriple(
            prefixes_input,
            std::make_tuple("<http://example.ontology.com/car#StateOfCharge>",
                            "<http://example.ontology.com/car#CurrentEnergy>",
                            "<http://www.w3.org/2001/XMLSchema#float>"),
            OBSERVATION_VALUE, TIMESTAMP);
    }
};

/**
 * @brief Test case for writing RDF triples in Turtle format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInTurtleFormat) {
    triple_writer->initiateTriple(VIN);
    SetUpRDFObjects(prefixes_fixture_);
    SetUpRDFData(prefixes_fixture_);

    std::string expected_RDF_triple = R"(@prefix car: <http://example.ontology.com/car#> .
@prefix sosa: <http://www.w3.org/ns/sosa/> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

car:Vehicle)" + VIN + R"(
	a car:Vehicle ;
	car:hasPart car:Powertrain)" + VIN +
                                      R"( .

car:Powertrain)" + VIN + R"(
	a car:Powertrain ;
	car:hasPart car:TractionBattery)" +
                                      VIN + R"( .

car:TractionBattery)" + VIN + R"(
	a car:TractionBattery ;
	car:hasSignal car:StateOfCharge)" +
                                      VIN + R"( .

car:StateOfCharge)" + VIN + R"(
	a car:StateOfCharge .

car:)" + OBSERVATION_IDENTIFIER + R"(
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:StateOfCharge)" +
                                      VIN + R"( ;
	sosa:hasSimpleResult ")" + OBSERVATION_VALUE +
                                      R"("^^xsd:float ;
	sosa:observedProperty car:CurrentEnergy ;
	sosa:phenomenonTime ")" + DATA_TIME +
                                      R"("^^xsd:dateTime .)";

    // Run and Assert
    std::string result_triple_writer =
        triple_writer->generateTripleOutput(ReasonerSyntaxType::TURTLE);
    ASSERT_EQ(result_triple_writer, expected_RDF_triple);
}

/**
 * @brief Test case for writing RDF triples in N-Triples format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInNTriplesFormat) {
    triple_writer->initiateTriple(VIN);
    SetUpRDFObjects(prefixes_fixture_);
    SetUpRDFData(prefixes_fixture_);

    std::string expected_RDF_triple =
        R"(<http://example.ontology.com/car#Vehicle)" + VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Vehicle> .
<http://example.ontology.com/car#Vehicle)" +
        VIN +
        R"(> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#Powertrain)" +
        VIN + R"(> .
<http://example.ontology.com/car#Powertrain)" +
        VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Powertrain> .
<http://example.ontology.com/car#Powertrain)" +
        VIN +
        R"(> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#TractionBattery)" +
        VIN + R"(> .
<http://example.ontology.com/car#TractionBattery)" +
        VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#TractionBattery> .
<http://example.ontology.com/car#TractionBattery)" +
        VIN +
        R"(> <http://example.ontology.com/car#hasSignal> <http://example.ontology.com/car#StateOfCharge)" +
        VIN + R"(> .
<http://example.ontology.com/car#StateOfCharge)" +
        VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#StateOfCharge> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://www.w3.org/ns/sosa/Observation> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER +
        R"(> <http://www.w3.org/ns/sosa/hasFeatureOfInterest> <http://example.ontology.com/car#StateOfCharge)" +
        VIN + R"(> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER + R"(> <http://www.w3.org/ns/sosa/hasSimpleResult> ")" +
        OBSERVATION_VALUE + R"("^^<http://www.w3.org/2001/XMLSchema#float> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER +
        R"(> <http://www.w3.org/ns/sosa/observedProperty> <http://example.ontology.com/car#CurrentEnergy> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER + R"(> <http://www.w3.org/ns/sosa/phenomenonTime> ")" + DATA_TIME +
        R"("^^<http://www.w3.org/2001/XMLSchema#dateTime> .)";

    // Run and Assert
    std::string result_triple_writer =
        triple_writer->generateTripleOutput(ReasonerSyntaxType::NTRIPLES);
    ASSERT_EQ(result_triple_writer, expected_RDF_triple);
}

/**
 * @brief Test case for writing RDF triples in N-Quads format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInNQuatdsFormat) {
    triple_writer->initiateTriple(VIN);
    SetUpRDFObjects(prefixes_fixture_);
    SetUpRDFData(prefixes_fixture_);

    std::string expected_RDF_triple =
        R"(<http://example.ontology.com/car#Vehicle)" + VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Vehicle> .
<http://example.ontology.com/car#Vehicle)" +
        VIN +
        R"(> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#Powertrain)" +
        VIN + R"(> .
<http://example.ontology.com/car#Powertrain)" +
        VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Powertrain> .
<http://example.ontology.com/car#Powertrain)" +
        VIN +
        R"(> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#TractionBattery)" +
        VIN + R"(> .
<http://example.ontology.com/car#TractionBattery)" +
        VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#TractionBattery> .
<http://example.ontology.com/car#TractionBattery)" +
        VIN +
        R"(> <http://example.ontology.com/car#hasSignal> <http://example.ontology.com/car#StateOfCharge)" +
        VIN + R"(> .
<http://example.ontology.com/car#StateOfCharge)" +
        VIN +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#StateOfCharge> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER +
        R"(> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://www.w3.org/ns/sosa/Observation> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER +
        R"(> <http://www.w3.org/ns/sosa/hasFeatureOfInterest> <http://example.ontology.com/car#StateOfCharge)" +
        VIN + R"(> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER + R"(> <http://www.w3.org/ns/sosa/hasSimpleResult> ")" +
        OBSERVATION_VALUE + R"("^^<http://www.w3.org/2001/XMLSchema#float> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER +
        R"(> <http://www.w3.org/ns/sosa/observedProperty> <http://example.ontology.com/car#CurrentEnergy> .
<http://example.ontology.com/car#)" +
        OBSERVATION_IDENTIFIER + R"(> <http://www.w3.org/ns/sosa/phenomenonTime> ")" + DATA_TIME +
        R"("^^<http://www.w3.org/2001/XMLSchema#dateTime> .)";

    // Run and Assert
    std::string result_triple_writer =
        triple_writer->generateTripleOutput(ReasonerSyntaxType::NQUADS);
    ASSERT_EQ(result_triple_writer, expected_RDF_triple);
}

/**
 * @brief Test case for writing RDF triples in TriG format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInTriGFormat) {
    triple_writer->initiateTriple(VIN);
    SetUpRDFObjects(prefixes_fixture_);
    SetUpRDFData(prefixes_fixture_);

    std::string expected_RDF_triple = R"(@prefix car: <http://example.ontology.com/car#> .
@prefix sosa: <http://www.w3.org/ns/sosa/> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

car:Vehicle)" + VIN + R"(
	a car:Vehicle ;
	car:hasPart car:Powertrain)" + VIN +
                                      R"( .

car:Powertrain)" + VIN + R"(
	a car:Powertrain ;
	car:hasPart car:TractionBattery)" +
                                      VIN + R"( .

car:TractionBattery)" + VIN + R"(
	a car:TractionBattery ;
	car:hasSignal car:StateOfCharge)" +
                                      VIN + R"( .

car:StateOfCharge)" + VIN + R"(
	a car:StateOfCharge .

car:)" + OBSERVATION_IDENTIFIER + R"(
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:StateOfCharge)" +
                                      VIN + R"( ;
	sosa:hasSimpleResult ")" + OBSERVATION_VALUE +
                                      R"("^^xsd:float ;
	sosa:observedProperty car:CurrentEnergy ;
	sosa:phenomenonTime ")" + DATA_TIME +
                                      R"("^^xsd:dateTime .)";
    // Run and Assert
    std::string result_triple_writer =
        triple_writer->generateTripleOutput(ReasonerSyntaxType::TRIG);
    ASSERT_EQ(result_triple_writer, expected_RDF_triple);
}

/**
 * @brief Test case for handling incorrect RDF prefix format.
 *
 * This test verifies that the system throws a runtime error when attempting
 * to set up RDF objects with prefixes that are incorrectly formatted.
 * It ensures that the TripleWriter correctly identifies and handles
 * malformed prefix strings.
 */
TEST_F(TripleWriterIntegrationTest, FailAddingRDFObjectToTripleSendingPrefixesWithWrongFormat) {
    // Define a string with incorrectly formatted RDF prefixes.
    const std::string wrong_prefixes = R"(prefix this_is_not_a_prefix
            prefix car: <http://example.ontology.com/car#>
            prefix middleware: <http://target-nameospace-for-data-middleware#>)";

    // Expect a runtime error to be thrown when setting up RDF objects with the wrong prefixes.
    EXPECT_THROW(SetUpRDFObjects(wrong_prefixes), std::runtime_error);
}

/**
 * @brief Test case for handling wrong data components.
 *
 * This test verifies that adding an RDF object to a triple with incorrect data components
 * results in a runtime error.
 */
TEST_F(TripleWriterIntegrationTest, FailAddingRDFObjectToTripleSendingWrongDataComponents) {
    const auto data_components =
        std::make_tuple("this_is_wrong", "<http://example.ontology.com/car#hasPart>",
                        "<http://example.ontology.com/car#Powertrain>");

    EXPECT_THROW(triple_writer->addElementObjectToTriple(prefixes_fixture_, data_components),
                 std::runtime_error);
}

/**
 * @brief Test case for handling incorrect RDF data setup with malformed prefixes.
 */
TEST_F(TripleWriterIntegrationTest, FailAddingRDFDataToTripleSendingPrefixesWithWrongFormat) {
    const std::string wrong_prefixes = R"(prefix this_is_not_a_prefix
            prefix car: <http://example.ontology.com/car#>
            prefix middleware: <http://target-nameospace-for-data-middleware#>)";
    EXPECT_THROW(SetUpRDFData(wrong_prefixes), std::runtime_error);
}

/**
 * @brief Test case for handling incorrect RDF data components in TripleWriter.
 *
 * This test verifies that attempting to add RDF data to a triple with incorrect or incomplete
 * data components results in a runtime error.
 */
TEST_F(TripleWriterIntegrationTest, FailAddingRDFDataToTripleSendingWrongDataComponents) {
    const auto data_components =
        std::make_tuple("", "<http://example.ontology.com/car#CurrentEnergy>",
                        "<http://www.w3.org/2001/XMLSchema#float>");

    EXPECT_THROW(triple_writer->addElementDataToTriple(prefixes_fixture_, data_components,
                                                       OBSERVATION_VALUE, TIMESTAMP),
                 std::runtime_error);
}
/**
 * @brief Test case for handling failure when adding RDF data with an incorrect data value.
 *
 * This test verifies that the TripleWriter throws a runtime error when attempting to add
 * RDF data to a triple with an empty data value.
 */
TEST_F(TripleWriterIntegrationTest, FailAddingRDFDataToTripleSendingWrongDataValue) {
    const auto data_components = std::make_tuple("<http://example.ontology.com/car#StateOfCharge>",
                                                 "<http://example.ontology.com/car#CurrentEnergy>",
                                                 "<http://www.w3.org/2001/XMLSchema#float>");

    EXPECT_THROW(
        triple_writer->addElementDataToTriple(prefixes_fixture_, data_components, "", TIMESTAMP),
        std::runtime_error);
}