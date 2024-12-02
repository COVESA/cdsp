#include <gtest/gtest.h>

#include <string>
#include <tuple>
#include <vector>

#include "triple_writer.h"

class TripleWriterIntegrationTest : public ::testing::Test {
   protected:
    TripleWriter* triple_writer;
    std::string prefixes_fixture;
    const std::string vin = "WBY11CF080CH470711";

    void SetUp() override {
        triple_writer = new TripleWriter();
        prefixes_fixture =
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
            triple_writer->addRDFObjectToTriple(prefixes_input, triple);
        }
    }

    void SetUpRDFData(const std::string& prefixes_input) {
        // RDF Data Triple
        triple_writer->addRDFDataToTriple(
            prefixes_input,
            std::make_tuple("<http://example.ontology.com/car#StateOfCharge>",
                            "<http://example.ontology.com/car#CurrentEnergy>",
                            "<http://www.w3.org/2001/XMLSchema#float>"),
            "98.6", "2018-11-16T15:50:27");
    }
};

/**
 * @brief Test case for writing RDF triples in Turtle format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInTurtleFormat) {
    triple_writer->initiateTriple(vin);
    SetUpRDFObjects(prefixes_fixture);
    SetUpRDFData(prefixes_fixture);

    std::string expected_RDF_triple = R"(@prefix car: <http://example.ontology.com/car#> .
@prefix sosa: <http://www.w3.org/ns/sosa/> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

car:VehicleWBY11CF080CH470711
	a car:Vehicle ;
	car:hasPart car:PowertrainWBY11CF080CH470711 .

car:PowertrainWBY11CF080CH470711
	a car:Powertrain ;
	car:hasPart car:TractionBatteryWBY11CF080CH470711 .

car:TractionBatteryWBY11CF080CH470711
	a car:TractionBattery ;
	car:hasSignal car:StateOfChargeWBY11CF080CH470711 .

car:StateOfChargeWBY11CF080CH470711
	a car:StateOfCharge .

car:Observation20181116155027O0
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:StateOfChargeWBY11CF080CH470711 ;
	sosa:hasSimpleResult "98.6"^^xsd:float ;
	sosa:observedProperty car:CurrentEnergy ;
	sosa:phenomenonTime "2018-11-16T15:50:27"^^xsd:dateTime .)";

    // Run and Assert
    std::string result_triple_writer = triple_writer->generateTripleOutput(RDFSyntaxType::TURTLE);
    ASSERT_EQ(result_triple_writer, expected_RDF_triple);
}

/**
 * @brief Test case for writing RDF triples in N-Triples format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInNTriplesFormat) {
    triple_writer->initiateTriple(vin);
    SetUpRDFObjects(prefixes_fixture);
    SetUpRDFData(prefixes_fixture);

    std::string expected_RDF_triple =
        R"(<http://example.ontology.com/car#VehicleWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Vehicle> .
<http://example.ontology.com/car#VehicleWBY11CF080CH470711> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#PowertrainWBY11CF080CH470711> .
<http://example.ontology.com/car#PowertrainWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Powertrain> .
<http://example.ontology.com/car#PowertrainWBY11CF080CH470711> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#TractionBatteryWBY11CF080CH470711> .
<http://example.ontology.com/car#TractionBatteryWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#TractionBattery> .
<http://example.ontology.com/car#TractionBatteryWBY11CF080CH470711> <http://example.ontology.com/car#hasSignal> <http://example.ontology.com/car#StateOfChargeWBY11CF080CH470711> .
<http://example.ontology.com/car#StateOfChargeWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#StateOfCharge> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://www.w3.org/ns/sosa/Observation> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/hasFeatureOfInterest> <http://example.ontology.com/car#StateOfChargeWBY11CF080CH470711> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/hasSimpleResult> "98.6"^^<http://www.w3.org/2001/XMLSchema#float> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/observedProperty> <http://example.ontology.com/car#CurrentEnergy> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/phenomenonTime> "2018-11-16T15:50:27"^^<http://www.w3.org/2001/XMLSchema#dateTime> .)";

    // Run and Assert
    std::string result_triple_writer = triple_writer->generateTripleOutput(RDFSyntaxType::NTRIPLES);
    ASSERT_EQ(result_triple_writer, expected_RDF_triple);
}

/**
 * @brief Test case for writing RDF triples in N-Quads format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInNQuatdsFormat) {
    triple_writer->initiateTriple(vin);
    SetUpRDFObjects(prefixes_fixture);
    SetUpRDFData(prefixes_fixture);

    std::string expected_RDF_triple =
        R"(<http://example.ontology.com/car#VehicleWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Vehicle> .
<http://example.ontology.com/car#VehicleWBY11CF080CH470711> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#PowertrainWBY11CF080CH470711> .
<http://example.ontology.com/car#PowertrainWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#Powertrain> .
<http://example.ontology.com/car#PowertrainWBY11CF080CH470711> <http://example.ontology.com/car#hasPart> <http://example.ontology.com/car#TractionBatteryWBY11CF080CH470711> .
<http://example.ontology.com/car#TractionBatteryWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#TractionBattery> .
<http://example.ontology.com/car#TractionBatteryWBY11CF080CH470711> <http://example.ontology.com/car#hasSignal> <http://example.ontology.com/car#StateOfChargeWBY11CF080CH470711> .
<http://example.ontology.com/car#StateOfChargeWBY11CF080CH470711> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://example.ontology.com/car#StateOfCharge> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://www.w3.org/ns/sosa/Observation> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/hasFeatureOfInterest> <http://example.ontology.com/car#StateOfChargeWBY11CF080CH470711> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/hasSimpleResult> "98.6"^^<http://www.w3.org/2001/XMLSchema#float> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/observedProperty> <http://example.ontology.com/car#CurrentEnergy> .
<http://example.ontology.com/car#Observation20181116155027O0> <http://www.w3.org/ns/sosa/phenomenonTime> "2018-11-16T15:50:27"^^<http://www.w3.org/2001/XMLSchema#dateTime> .)";

    // Run and Assert
    std::string result_triple_writer = triple_writer->generateTripleOutput(RDFSyntaxType::NQUADS);
    ASSERT_EQ(result_triple_writer, expected_RDF_triple);
}

/**
 * @brief Test case for writing RDF triples in TriG format.
 */
TEST_F(TripleWriterIntegrationTest, WriteRDFTripleInTriGFormat) {
    triple_writer->initiateTriple(vin);
    SetUpRDFObjects(prefixes_fixture);
    SetUpRDFData(prefixes_fixture);

    std::string expected_RDF_triple = R"(@prefix car: <http://example.ontology.com/car#> .
@prefix sosa: <http://www.w3.org/ns/sosa/> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .

car:VehicleWBY11CF080CH470711
	a car:Vehicle ;
	car:hasPart car:PowertrainWBY11CF080CH470711 .

car:PowertrainWBY11CF080CH470711
	a car:Powertrain ;
	car:hasPart car:TractionBatteryWBY11CF080CH470711 .

car:TractionBatteryWBY11CF080CH470711
	a car:TractionBattery ;
	car:hasSignal car:StateOfChargeWBY11CF080CH470711 .

car:StateOfChargeWBY11CF080CH470711
	a car:StateOfCharge .

car:Observation20181116155027O0
	a sosa:Observation ;
	sosa:hasFeatureOfInterest car:StateOfChargeWBY11CF080CH470711 ;
	sosa:hasSimpleResult "98.6"^^xsd:float ;
	sosa:observedProperty car:CurrentEnergy ;
	sosa:phenomenonTime "2018-11-16T15:50:27"^^xsd:dateTime .)";
    // Run and Assert
    std::string result_triple_writer = triple_writer->generateTripleOutput(RDFSyntaxType::TRIG);
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

    EXPECT_THROW(triple_writer->addRDFObjectToTriple(prefixes_fixture, data_components),
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

    EXPECT_THROW(triple_writer->addRDFDataToTriple(prefixes_fixture, data_components, "98.6",
                                                   "2018-11-16T15:50:27"),
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

    EXPECT_THROW(triple_writer->addRDFDataToTriple(prefixes_fixture, data_components, "",
                                                   "2018-11-16T15:50:27"),
                 std::runtime_error);
}