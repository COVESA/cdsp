#include <serd/serd.h>

#include <iostream>
#include <string>

// Global string to store the Serd output
std::string serd_output;

// Custom write function to append output to the serd_output string
unsigned long write_to_string(const void* buf, unsigned long len, void* stream) {
    serd_output.append(static_cast<const char*>(buf), len);
    return len;
}

int main() {
    // Create a Serd environment for writing Turtle format
    SerdEnv* env = serd_env_new(nullptr);

    // Create the writer with the output directed to write_to_string
    SerdWriter* writer = serd_writer_new(SERD_TURTLE, SERD_STYLE_ABBREVIATED, env, nullptr,
                                         write_to_string, nullptr);

    // Declare namespaces
    SerdNode car_prefix = serd_node_from_string(SERD_CURIE, (const uint8_t*) "car");
    SerdNode car_uri =
        serd_node_from_string(SERD_URI, (const uint8_t*) "http://example.ontology.com/car#");
    serd_writer_set_prefix(writer, &car_prefix, &car_uri);

    SerdNode xsd_prefix = serd_node_from_string(SERD_CURIE, (const uint8_t*) "xsd");
    SerdNode xsd_uri =
        serd_node_from_string(SERD_URI, (const uint8_t*) "http://www.w3.org/2001/XMLSchema#");
    serd_writer_set_prefix(writer, &xsd_prefix, &xsd_uri);

    SerdNode sosa_prefix = serd_node_from_string(SERD_CURIE, (const uint8_t*) "sosa");
    SerdNode sosa_uri =
        serd_node_from_string(SERD_URI, (const uint8_t*) "http://www.w3.org/ns/sosa/");
    serd_writer_set_prefix(writer, &sosa_prefix, &sosa_uri);

    SerdNode other_prefix = serd_node_from_string(SERD_CURIE, (const uint8_t*) "other");
    SerdNode other_uri = serd_node_from_string(SERD_URI, (const uint8_t*) "http://www.other.com/");
    serd_writer_set_prefix(writer, &other_prefix, &other_uri);

    // Define URIs and literals for the triples
    SerdNode vehicle_subject =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:VehicleWBY11CF080CH470711");
    SerdNode type_predicate = serd_node_from_string(
        SERD_URI, (const uint8_t*) "http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
    SerdNode vehicle_object = serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:Vehicle");

    SerdNode has_part = serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:hasPart");
    SerdNode powertrain_object =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:PowertrainWBY11CF080CH470711");
    SerdNode powertrain_type = serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:Powertrain");

    SerdNode transmission_object =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:TransmissionWBY11CF080CH470711");
    SerdNode transmission_type =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:Transmission");

    // Observation triples
    std::string test = "car:Observation20181116155027";

    SerdNode observation = serd_node_from_string(SERD_CURIE, (const uint8_t*) test.c_str());
    SerdNode observation_type =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "sosa:Observation");
    SerdNode feature_of_interest =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "sosa:hasFeatureOfInterest");
    SerdNode observed_property =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "sosa:observedProperty");
    SerdNode current_energy =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "car:CurrentEnergy");
    SerdNode has_result =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "sosa:hasSimpleResult");
    SerdNode result_value = serd_node_from_string(SERD_LITERAL, (const uint8_t*) "98.6");
    SerdNode xsd_float = serd_node_from_string(SERD_CURIE, (const uint8_t*) "xsd:float");
    SerdNode phenomenon_time =
        serd_node_from_string(SERD_CURIE, (const uint8_t*) "sosa:phenomenonTime");
    SerdNode time_value = serd_node_from_string(
        SERD_LITERAL, (const uint8_t*) "\"2018-11-16 15:50:27\"^^xsd:dateTime");

    // Write triples for vehicle and parts
    serd_writer_write_statement(writer, 0, nullptr, &vehicle_subject, &type_predicate,
                                &vehicle_object, nullptr, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &vehicle_subject, &has_part, &powertrain_object,
                                nullptr, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &powertrain_object, &type_predicate,
                                &powertrain_type, nullptr, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &powertrain_object, &has_part,
                                &transmission_object, nullptr, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &transmission_object, &type_predicate,
                                &transmission_type, nullptr, nullptr);

    // Write observation triples
    serd_writer_write_statement(writer, 0, nullptr, &observation, &type_predicate,
                                &observation_type, nullptr, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &observation, &feature_of_interest,
                                &transmission_object, nullptr, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &observation, &has_result, &result_value,
                                &xsd_float, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &observation, &observed_property,
                                &current_energy, nullptr, nullptr);
    serd_writer_write_statement(writer, 0, nullptr, &observation, &phenomenon_time, &time_value,
                                nullptr, nullptr);

    // Finish and release resources
    serd_writer_finish(writer);
    serd_writer_free(writer);
    serd_env_free(env);

    // Output the result
    std::cout << "Generated RDF triples:\n" << serd_output << std::endl;

    return 0;
}