#include "triple_writer.h"

#include <ctime>
#include <iostream>
#include <sstream>

#include "helper.h"

// Global string to store the Serd output
std::string serd_output;

/**
 * @brief Writes serialized data to a string.
 *
 * This function appends the serialized data from the buffer to the internal
 * string representation. It is used to capture the output of a serialization
 * process into a string.
 *
 * @param buf Pointer to the buffer containing the serialized data.
 * @param len The length of the data in the buffer.
 * @param stream A pointer to the stream, not used in this implementation.
 * @return The number of bytes written to the string, which is equal to len.
 */
unsigned long write_serd_output_to_string(const void* buf, unsigned long len, void* stream) {
    serd_output.append(static_cast<const char*>(buf), len);
    return len;
}

/**
 * @brief Initiates the TripleWriter.
 *
 * This function initiates and assigns a new identifier to the triple.
 *
 * @param identifier The new identifier for the triple.
 *
 * @throws std::runtime_error If the provided identifier is an empty string.
 */
void TripleWriter::initiateTriple(const std::string& identifier) {
    if (identifier.empty())
        throw std::runtime_error("Triple identifier cannot be empty");
    identifier_ = identifier;
    rdf_triples_definitions_.clear();
    unique_rdf_prefix_definitions_.clear();
}

/**
 * @brief Adds an RDF object to a triple by processing its components.
 *
 * This function takes RDF object values and prefixes, processes them to extract
 * necessary components, and constructs RDF triples. The triples are then added
 * to the internal list of triple definitions.
 *
 * @param prefixes A string containing the prefixes to be added to the system list.
 * @param rdf_object_values A tuple containing three strings representing RDF object values:
 *                          - The first element is the class 1 RDF element.
 *                          - The second element is the object property RDF element.
 *                          - The third element is the class 2 RDF element.
 */
void TripleWriter::addElementObjectToTriple(
    const std::string& prefixes,
    const std::tuple<std::string, std::string, std::string>& rdf_object_values) {
    // Add all prefixes in the system list
    addSuportedPrefixes(prefixes);

    // Split prefix and identifier for each RDF object value
    const auto [class_1_prefix, class_1_identifier] =
        extractPrefixAndIdentifierFromRdfElement(std::get<0>(rdf_object_values));
    const auto [object_property_prefix, object_property_identifier] =
        extractPrefixAndIdentifierFromRdfElement(std::get<1>(rdf_object_values));
    const auto [class_2_prefix, class_2_identifier] =
        extractPrefixAndIdentifierFromRdfElement(std::get<2>(rdf_object_values));

    // Create identifiers instances for each class
    const std::string class_1_instance_uri = createInstanceUri(class_1_prefix, class_1_identifier);
    const std::string class_2_instance_uri = createInstanceUri(class_2_prefix, class_2_identifier);

    TripleNodes triple_nodes;

    // Create triples
    triple_nodes.subject = std::make_pair(SERD_CURIE, class_1_instance_uri);
    triple_nodes.predicate =
        std::make_pair(SERD_URI, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
    triple_nodes.object = std::make_pair(SERD_CURIE, class_1_prefix + ":" + class_1_identifier);
    rdf_triples_definitions_.push_back(triple_nodes);

    triple_nodes.predicate =
        std::make_pair(SERD_CURIE, object_property_prefix + ":" + object_property_identifier);
    triple_nodes.object = std::make_pair(SERD_CURIE, class_2_instance_uri);
    rdf_triples_definitions_.push_back(triple_nodes);
}

/**
 * @brief Adds RDF data to a triple store with specified prefixes and RDF data values.
 *
 * This function constructs RDF triples using the provided RDF data values and prefixes,
 * and adds them to the triple store. It also handles specific cases for certain RDF data
 * values, such as "CurrentLocation" with "Latitude" or "Longitude", requiring an NTM value.
 *
 * @param prefixes A string containing the prefixes to be added to the system list.
 * @param rdf_data_values A tuple containing three strings representing RDF data values:
 *                        - The first element is the class identifier.
 *                        - The second element is the data property identifier.
 *                        - The third element is the data type identifier.
 * @param value A string representing the value to be used in the RDF triple.
 * @param timestamp A time_point object representing the timestamp for the RDF data.
 * @param ntmValue An optional double representing the NTM value, required for specific cases.
 *
 * @throws std::runtime_error If the value is empty, or if the NTM value is required
 *                            but not provided.
 */
void TripleWriter::addElementDataToTriple(
    const std::string& prefixes,
    const std::tuple<std::string, std::string, std::string>& rdf_data_values,
    const std::string& value, const std::chrono::system_clock::time_point& timestamp,
    const std::optional<double>& ntmValue) {
    if (value.empty()) {
        throw std::runtime_error("Triple value cannot be empty");
    }
    // Add all prefixes in the system list
    addSuportedPrefixes(prefixes);

    // Insert a prefix to the definitions list that will be used for the observation
    unique_rdf_prefix_definitions_.emplace("sosa", "http://www.w3.org/ns/sosa/");
    unique_rdf_prefix_definitions_.emplace("xsd", "http://www.w3.org/2001/XMLSchema#");

    // Split prefix and identifier for each RDF data value
    const auto [class_1_prefix, class_1_identifier] =
        extractPrefixAndIdentifierFromRdfElement(std::get<0>(rdf_data_values));
    const auto [data_property_prefix, data_property_identifier] =
        extractPrefixAndIdentifierFromRdfElement(std::get<1>(rdf_data_values));
    const auto [data_type_prefix, data_type_identifier] =
        extractPrefixAndIdentifierFromRdfElement(std::get<2>(rdf_data_values));

    TripleNodes triple_nodes;

    // Create identifiers instances for each class

    const std::string date_time_with_nano =
        Helper::getFormattedTimestampCustom("%Y-%m-%dT%H:%M:%S", timestamp, true);

    const std::string observation_identifier =
        Helper::getFormattedTimestampCustom("%Y%m%d%H%M%S", timestamp) +
        Helper::extractNanoseconds(timestamp);

    auto milliseconds_since_epoch =
        std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();

    const std::string class_1_instance_uri = createInstanceUri(class_1_prefix, class_1_identifier);
    std::stringstream observation_instance_uri;
    observation_instance_uri << class_1_prefix << ":" << "ob_" << data_property_identifier << "_"
                             << observation_identifier;

    // Create triples
    triple_nodes.subject = std::make_pair(SERD_CURIE, class_1_instance_uri);
    triple_nodes.predicate =
        std::make_pair(SERD_URI, "http://www.w3.org/1999/02/22-rdf-syntax-ns#type");
    triple_nodes.object = std::make_pair(SERD_CURIE, class_1_prefix + ":" + class_1_identifier);
    rdf_triples_definitions_.push_back(triple_nodes);

    triple_nodes.subject = std::make_pair(SERD_CURIE, observation_instance_uri.str());
    triple_nodes.object = std::make_pair(SERD_CURIE, "sosa:Observation");
    rdf_triples_definitions_.push_back(triple_nodes);

    triple_nodes.predicate = std::make_pair(SERD_CURIE, "sosa:hasFeatureOfInterest");
    triple_nodes.object = std::make_pair(SERD_CURIE, class_1_instance_uri);
    rdf_triples_definitions_.push_back(triple_nodes);

    triple_nodes.predicate = std::make_pair(SERD_CURIE, "sosa:hasSimpleResult");
    triple_nodes.object = std::make_pair(SERD_LITERAL, value);
    triple_nodes.datatype =
        std::make_pair(SERD_CURIE, data_type_prefix + ":" + data_type_identifier);
    rdf_triples_definitions_.push_back(triple_nodes);

    triple_nodes.predicate = std::make_pair(SERD_CURIE, "sosa:observedProperty");
    triple_nodes.object =
        std::make_pair(SERD_CURIE, data_property_prefix + ":" + data_property_identifier);
    triple_nodes.datatype = std::nullopt;
    rdf_triples_definitions_.push_back(triple_nodes);

    triple_nodes.predicate = std::make_pair(SERD_CURIE, "sosa:phenomenonTime");
    triple_nodes.object = std::make_pair(SERD_LITERAL, date_time_with_nano);
    triple_nodes.datatype = std::make_pair(SERD_CURIE, "xsd:dateTime");
    rdf_triples_definitions_.push_back(triple_nodes);

    if (class_1_identifier == "CurrentLocation" &&
        (data_property_identifier == "latitude" || data_property_identifier == "longitude")) {
        if (!ntmValue.has_value()) {
            throw std::runtime_error("NTM value cannot be empty");
        }

        auto ntmValue_str = std::to_string(ntmValue.value());
        triple_nodes.predicate = std::make_pair(SERD_CURIE, class_1_prefix + ":hasSimpleResultNTM");
        triple_nodes.object = std::make_pair(SERD_LITERAL, ntmValue_str);
        triple_nodes.datatype =
            std::make_pair(SERD_CURIE, data_type_prefix + ":" + data_type_identifier);
        rdf_triples_definitions_.push_back(triple_nodes);
    }
}

/**
 * @brief Generates a serialized RDF triple output in the specified format.
 *
 * This function creates a new Serd environment and writer to serialize RDF triples
 * into a string format specified by the input parameter. It sets up the necessary
 * namespaces and writes the triples using the Serd library.
 *
 * @param format The RDF syntax type to use for serialization.
 * @return A string containing the serialized RDF triples.
 */
std::string TripleWriter::generateTripleOutput(const ReasonerSyntaxType& format) {
    serd_output.clear();

    SerdEnv* serd_env = serd_env_new(nullptr);
    SerdSyntax serd_format = getSerdSyntax(format);

    // Create the writer with the base URI
    SerdWriter* serd_writer = serd_writer_new(serd_format, SERD_STYLE_ABBREVIATED, serd_env,
                                              nullptr, write_serd_output_to_string, this);

    // Declare namespaces
    for (const auto& [prefix, uri] : unique_rdf_prefix_definitions_) {
        SerdNode car_prefix = serd_node_from_string(SERD_CURIE, (const uint8_t*) prefix.c_str());
        SerdNode car_uri = serd_node_from_string(SERD_URI, (const uint8_t*) uri.c_str());
        serd_writer_set_prefix(serd_writer, &car_prefix, &car_uri);
    }

    // Write triples
    for (const TripleNodes& triple_nodes : rdf_triples_definitions_) {
        SerdNode subject_node = serd_node_from_string(
            triple_nodes.subject.first, (const uint8_t*) triple_nodes.subject.second.c_str());
        SerdNode predicate_node = serd_node_from_string(
            triple_nodes.predicate.first, (const uint8_t*) triple_nodes.predicate.second.c_str());
        SerdNode object_node = serd_node_from_string(
            triple_nodes.object.first, (const uint8_t*) triple_nodes.object.second.c_str());

        const SerdNode* datatype_node_ptr = nullptr;
        if (triple_nodes.datatype.has_value()) {
            std::pair<SerdType, std::string> datatype = triple_nodes.datatype.value();
            SerdNode datatype_node =
                serd_node_from_string(datatype.first, (const uint8_t*) datatype.second.c_str());
            datatype_node_ptr = &datatype_node;
        }
        serd_writer_write_statement(serd_writer, 0, nullptr, &subject_node, &predicate_node,
                                    &object_node, datatype_node_ptr, nullptr);
    }

    // End the document
    serd_writer_finish(serd_writer);

    // Free resources
    serd_writer_free(serd_writer);
    serd_env_free(serd_env);

    return Helper::trimTrailingNewlines(serd_output);
}

/**
 * @brief Converts an RDF syntax type to the corresponding Serd syntax.
 *
 * @param format The RDF syntax type to be converted.
 * @return The corresponding Serd syntax type.
 * @throws std::runtime_error if the RDF syntax type is unsupported.
 */
SerdSyntax TripleWriter::getSerdSyntax(const ReasonerSyntaxType& format) {
    switch (format) {
        case ReasonerSyntaxType::TURTLE:
            return SerdSyntax::SERD_TURTLE;
            break;
        case ReasonerSyntaxType::NQUADS:
            return SerdSyntax::SERD_NQUADS;
            break;
        case ReasonerSyntaxType::NTRIPLES:
            return SerdSyntax::SERD_NTRIPLES;
            break;
        case ReasonerSyntaxType::TRIG:
            return SerdSyntax::SERD_TRIG;
            break;
        default:
            throw std::runtime_error("Unsupported Serd syntax format");
    }
}

/**
 * @brief Adds prefixes to the internal supported prefixes storage from a given string.
 *
 * This function processes a string containing multiple prefix definitions,
 * each on a new line, and extracts the prefix and its corresponding URI using
 * a regular expression. The extracted prefixes and URIs are then stored in
 * an internal data structure for later use.
 *
 * @param prefixes A string containing prefix definitions, each in the format
 * "prefix <prefix_name>: <URI>", separated by new lines.
 */
void TripleWriter::addSuportedPrefixes(const std::string& prefixes) {
    if (!prefixes.empty()) {
        // Regular expression to capture prefix and URI
        std::regex pattern(R"(prefix\s+(\w+):\s+<([^>]+)>)");

        std::istringstream stream(prefixes);
        std::string line;

        while (std::getline(stream, line)) {
            auto [prefix, uri] = extractTupleFromString(pattern, line);
            unique_supported_prefixes_.emplace(prefix, uri);
        }
    } else {
        throw std::runtime_error("Prefixes cannot be empty");
    }
}

/**
 * @brief Extracts namespace prefix and identifier value from an RDF element.
 *
 * @param element RDF element string in angle brackets.
 * @return Tuple of namespace prefix and identifier value.
 * @throws std::runtime_error If format is unsupported.
 */
std::tuple<std::string, std::string> TripleWriter::extractPrefixAndIdentifierFromRdfElement(
    const std::string& element) {
    if (!element.empty()) {
        // Regular expression to capture prefix and URI
        std::regex pattern(R"(http://([^#]+)#([^>]+)>)");

        auto [rdf_namespace, identifier] = extractTupleFromString(pattern, element);
        addTriplePrefix(rdf_namespace);
        return std::make_tuple(rdf_namespace, identifier);
    } else {
        throw std::runtime_error("The RDF element cannot be empty");
    }
}

/**
 * @brief Adds a prefix to the triple prefixes if it matches a URI.
 *
 * This function searches through the unique supported prefixes and checks if the given URI contains
 * the specified prefix. If a match is found, the prefix and URI are added to the unique tuple
 * prefixes.
 *
 * @param prefix The prefix to be added to the triple prefixes.
 */
void TripleWriter::addTriplePrefix(std::string& prefix) {
    std::string found_prefix;

    // Search and add a URI and prefix to the triple prefixes
    for (const auto& [system_prefix, uri] : unique_supported_prefixes_) {
        if (uri.find(prefix) != std::string::npos) {
            unique_rdf_prefix_definitions_.emplace(system_prefix, uri);
            prefix = system_prefix;
            break;
        }
    }
}

/**
 * @brief Creates a URI for an instance using the provided prefix and name.
 *
 * @param prefix The prefix to be used in the URI.
 * @param name The name to be used in the URI.
 * @return A string representing the constructed URI.
 * @throws std::runtime_error if the triple identifier has not been set.
 */
std::string TripleWriter::createInstanceUri(const std::string& prefix, const std::string& name) {
    if (identifier_.empty()) {
        throw std::runtime_error("Triple identifier has not been set");
    }
    return prefix + ":" + name + identifier_;
}

/**
 * @brief Extracts a tuple of strings from a given input string using a regex pattern.
 *
 * @param pattern The regex pattern used to search the input string. It should be designed to
 * capture two groups.
 * @param value The input string to be searched using the regex pattern.
 * @return A tuple containing the two captured groups as strings.
 * @throws std::runtime_error If the input string does not match the pattern or does not capture
 * exactly two groups.
 */
std::tuple<std::string, std::string> TripleWriter::extractTupleFromString(std::regex pattern,
                                                                          std::string value) {
    std::smatch matches;
    // Check if the pattern matches
    if (std::regex_search(value, matches, pattern) && matches.size() == 3) {
        return std::make_tuple(matches[1], matches[2]);
    } else {
        throw std::runtime_error("Unsupported input format: " + value);
    }
}
