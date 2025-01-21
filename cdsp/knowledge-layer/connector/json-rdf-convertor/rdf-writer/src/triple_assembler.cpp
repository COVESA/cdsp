#include "triple_assembler.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "helper.h"

using json = nlohmann::json;

TripleAssembler::TripleAssembler(const ModelConfig& model_config, RDFoxAdapter& adapter,
                                 IFileHandler& file_reader, TripleWriter& triple_writer)
    : model_config_(model_config),
      rdfox_adapter_(adapter),
      file_handler_(file_reader),
      triple_writer_(triple_writer) {}

/**
 * @brief Initializes the TripleAssembler by checking the data store and loading SHACL shapes.
 *
 * @throws std::runtime_error If the data store is unavailable or if SHACL shapes cannot be loaded.
 */
void TripleAssembler::initialize() {
    if (!rdfox_adapter_.checkDataStore()) {
        throw std::runtime_error("Initialization failed: Unable to generate triples.");
    }
    if (!model_config_.shacl_shapes_files.empty()) {
        for (const auto& file : model_config_.shacl_shapes_files) {
            const std::string ttl_data = file_handler_.readFile(file);
            if (ttl_data.empty() || !rdfox_adapter_.loadData(ttl_data)) {
                throw std::runtime_error(
                    "No SHACL shapes could be loaded. The triples cannot be generated.");
            }
        }
    } else {
        throw std::runtime_error(
            "No SHACL shapes were found to load. The triples cannot be generated.");
    }
}

/**
 * Transforms a DataMessage into RDF triples and stores them.
 *
 * This function checks the availability of the RDFox datastore and, if available,
 * processes each node in the DataMessage to generate RDF triples. The generated
 * triples are then stored using the triple writer.
 *
 * @param message The DataMessage containing nodes and metadata to be transformed into RDF triples.
 * @throws std::runtime_error If the datastore is not available.
 */
void TripleAssembler::transformMessageToRDFTriple(const DataMessage& message) {
    if (!rdfox_adapter_.checkDataStore()) {
        throw std::runtime_error("Failed to call datastore. The triples cannot be generated.");
    }

    // Add the identifier to the triples that will be generated
    triple_writer_.initiateTriple(message.header.id);
    std::string msg_tree = Helper::toLowerCase(message.header.tree);
    if (message.nodes.empty()) {
        std::cout << "No nodes found in the message\n\n";
        return;
    }

    std::optional<CoordinateNodes> valid_coordinates = std::nullopt;

    for (const auto node : message.nodes) {
        if (node.name == "Vehicle.CurrentLocation.Latitude" ||
            node.name == "Vehicle.CurrentLocation.Longitude") {
            const auto epochMilliseconds =
                Helper::getMillisecondsSinceEpoch(message.header.date_time);
            const auto found_key = timestamp_coordinates_messages_map_.find(epochMilliseconds);
            if (found_key != timestamp_coordinates_messages_map_.end()) {
                found_key->second.insert({node.name, node});
            } else {
                timestamp_coordinates_messages_map_.emplace(
                    epochMilliseconds, std::unordered_map<std::string, Node>{{node.name, node}});
            }
            valid_coordinates = getValidCoordinatesPair();

        } else {
            try {
                generateTriplesFromNode(node, msg_tree, message.header.date_time);
            } catch (const std::exception& e) {
                std::cerr << "An error occurred creating the TTL triples: " << e.what()
                          << std::endl;
            }
        }
    }

    if (valid_coordinates.has_value())
        generateTriplesFromCoordinates(valid_coordinates, msg_tree, message);

    // Get the document of the generated triples
    std::string generated_triples =
        triple_writer_.generateTripleOutput(model_config_.reasoner_settings.output_format);

    if (!generated_triples.empty()) {
        storeTripleOutput(generated_triples);
    } else {
        std::cout << "Any triples have been generated for the update message\n\n";
    }
}

/**
 * Retrieves a valid pair of latitude and longitude coordinates from the timestamped coordinate
 * messages.
 *
 * This function iterates over a map of timestamped coordinate messages to find the most recent
 * valid pair of latitude and longitude nodes. A valid pair is defined as having both latitude and
 * longitude nodes occurring within a 2-second window.
 *
 * @return An optional containing a CoordinateNodes object if a valid pair is found, or std::nullopt
 * if no valid pair exists.
 */
std::optional<CoordinateNodes> TripleAssembler::getValidCoordinatesPair() {
    chrono_time_mili last_timestamp_to_delete(0.0);
    const Node* latitude = nullptr;
    chrono_time_mili latitude_time;
    const Node* longitude = nullptr;
    chrono_time_mili longitude_time;

    for (const auto& [time, nodes] : timestamp_coordinates_messages_map_) {
        // Check if latitude exists
        if (const auto found_lat = nodes.find("Vehicle.CurrentLocation.Latitude");
            found_lat != nodes.end()) {
            latitude_time = time;
            latitude = &found_lat->second;
        }

        // Check if longitude exists
        if (const auto found_long = nodes.find("Vehicle.CurrentLocation.Longitude");
            found_long != nodes.end()) {
            longitude_time = time;
            longitude = &found_long->second;
        }
    }

    if (latitude && longitude) {
        // Ensure that the locations happen within a period of 2 seconds
        const auto two_seconds_in_millisec = std::chrono::duration<double, std::milli>(2000.0);
        auto difference = latitude_time - longitude_time;

        if (std::abs(difference.count()) <= two_seconds_in_millisec.count()) {
            // Update the last timestamp for valid coordinates
            coordinates_last_time_stamp_ = std::max(latitude_time, longitude_time);
            return CoordinateNodes{*latitude, *longitude};
        }
    }

    return std::nullopt;
}

/**
 * @brief Cleans up old timestamps from the timestamp_coordinates_messages_map_.
 *
 * This function iterates through the timestamp_coordinates_messages_map_ and removes
 * entries where the timestamp (key) is less or equal than the coordinates_last_time_stamp_
 * (coordinates already inserted). It ensures that only relevant and recent coordinates entries are
 * retained in the map.
 */
void TripleAssembler::cleanupOldTimestamps() {
    for (auto it = timestamp_coordinates_messages_map_.begin();
         it != timestamp_coordinates_messages_map_.end();) {
        if (it->first <= coordinates_last_time_stamp_) {
            it = timestamp_coordinates_messages_map_.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * @brief Generates RDF triples from a given node.
 *
 * This function processes a node by extracting its object and data elements,
 * querying necessary prefixes and values, and then adding these as RDF objects
 * and data to the triple writer. It handles specific node names
 * related to vehicle location by preparing additional RDF data if necessary.
 *
 * @param node The node containing the name and value to be transformed into RDF triples.
 * @param msg_tree The message tree structure used for querying RDF data.
 * @param msg_date_time The date and time associated with the message.
 * @param ntm_coord_value An optional coordinate value for NTM data.
 */
void TripleAssembler::generateTriplesFromNode(const Node& node, const std::string& msg_tree,
                                              const std::string& msg_date_time,
                                              const std::optional<double>& ntm_coord_value) {
    // Split node data point into object and data elements
    const auto [object_elements, data_element] = extractObjectsAndDataElements(node.name);

    // Query and add RDF Objects
    for (std::size_t i = 1; i < object_elements.size(); ++i) {
        const auto [prefixes, rdf_object_values] =
            getQueryPrefixesAndData(msg_tree, "object", object_elements[i - 1], object_elements[i]);

        triple_writer_.addRDFObjectToTriple(prefixes, rdf_object_values);
    }

    // Query and add RDF Data
    const auto [prefixes, rdf_data_values] = getQueryPrefixesAndData(
        msg_tree, "data", object_elements[object_elements.size() - 1], data_element);

    triple_writer_.addRDFDataToTriple(prefixes, rdf_data_values, node.value, msg_date_time,
                                      ntm_coord_value);
}

/**
 * @brief Retrieves a valid pair of latitude and longitude coordinates.
 *
 * This function iterates through the `timestamp_coordinates_messages_map_` to find
 * the most recent latitude and longitude nodes. It checks if both coordinates exist
 * and ensures they are recorded within a 2-second interval. If valid, it updates
 * the `coordinates_last_time_stamp_` and returns the coordinate nodes.
 *
 * @return An optional CoordinateNodes object containing the latitude and longitude nodes
 * if both are found and valid; otherwise, std::nullopt.
 */
void TripleAssembler::generateTriplesFromCoordinates(
    std::optional<CoordinateNodes>& valid_coordinates, std::string& msg_tree,
    const DataMessage& message) {
    {
        try {
            auto ntm_coord = Helper::getCoordInNtm(valid_coordinates.value().latitude.value,
                                                   valid_coordinates.value().longitude.value);
            if (ntm_coord == std::nullopt) {
                throw std::runtime_error("Failed to convert coordinates to NTM");
            }

            generateTriplesFromNode(valid_coordinates.value().latitude, msg_tree,
                                    message.header.date_time, ntm_coord.value().northing);
            generateTriplesFromNode(valid_coordinates.value().longitude, msg_tree,
                                    message.header.date_time, ntm_coord.value().easting);
        } catch (const std::exception& e) {
            std::cerr << "An error occurred creating the TTL triples: " << e.what() << std::endl;
        }

        // Manual cleanup of old timestamps
        cleanupOldTimestamps();
    }
}

/**
 * Extracts objects and data elements from a given node name.
 *
 * @param node_name The node name to be processed, expected to be in the format
 *                  of dot-separated elements.
 * @return A pair where the first element is a vector of strings representing
 *         the objects, and the second element is a string representing the
 *         data element.
 * @throws std::runtime_error if the node name contains fewer than two elements.
 */
std::pair<std::vector<std::string>, std::string> TripleAssembler::extractObjectsAndDataElements(
    const std::string& node_name) {
    std::vector<std::string> elements;
    std::stringstream ss(node_name);
    std::string token;

    // Split the node name by dots
    while (std::getline(ss, token, '.')) {
        elements.push_back(token);
    }

    // At least two elements are required to create a triple
    if (elements.size() < 2) {
        throw std::runtime_error("The message node must contain at least two elements: " +
                                 node_name);
    }
    // Separate the last element as `data_element`
    std::string data_element = elements.back();
    elements.pop_back();

    return {elements, data_element};
}

/**
 * @brief Retrieves SPARQL query prefixes and data elements for RDF triple generation.
 *
 * This method constructs a SPARQL query based on the provided message tree and property type.
 * It attempts to retrieve the query from a specified file path. If the query is not found,
 * it defaults to a query from a "default" path.
 *
 * @param message_tree The path or identifier for the message tree to query.
 * @param property_type The type of property ("object" or "data") to query.
 * @param subject_class The class name of the subject in the RDF triple.
 * @param object_class The class name of the object in the RDF triple.
 * @return A pair consisting of:
 *         - A string containing the extracted prefixes from the SPARQL query.
 *         - A tuple containing the extracted three elements from the query result.
 *
 * @throws std::runtime_error If the SPARQL query cannot be retrieved or executed.
 */
std::pair<std::string, std::tuple<std::string, std::string, std::string>>
TripleAssembler::getQueryPrefixesAndData(const std::string& message_tree,
                                         const std::string& property_type,
                                         const std::string& subject_class,
                                         const std::string& object_class) {
    std::string sparql_query = getQueryFromFilePath(message_tree, property_type);

    if (sparql_query.empty()) {
        sparql_query = getQueryFromFilePath("default", property_type);
    }

    if (sparql_query.empty()) {
        throw std::runtime_error("Failed to querying " + property_type +
                                 " properties in the model config. The triples cannot be "
                                 "generated.");
    }

    replaceAllSparqlVariables(sparql_query, "%A%", subject_class);
    replaceAllSparqlVariables(sparql_query, "%B%", object_class);

    const std::string query_result = rdfox_adapter_.queryData(sparql_query);
    const auto element_values = extractElementValuesFromQuery(query_result);
    const std::string prefixes = extractPrefixesFromQuery(sparql_query);
    return std::make_pair(prefixes, element_values);
}

/**
 * @brief Retrieves a SPARQL query from a file based on the specified tree and property type.
 *
 * @param tree The identifier for the message tree to search for query files.
 * @param property_type The type of property for which the query is needed.
 * @return A string containing the SPARQL query if found, or an empty string if no matching file is
 * found.
 */
std::string TripleAssembler::getQueryFromFilePath(const std::string& tree,
                                                  const std::string& property_type) {
    auto it = model_config_.triple_assembler_queries_files.find(tree);
    if (it != model_config_.triple_assembler_queries_files.end()) {
        const std::vector<std::string>& files = it->second;
        for (const auto& file : files) {
            if (file.find(property_type + "_property") != std::string::npos) {
                return file_handler_.readFile(file);
            }
        }
    }
    return "";
}

/**
 * @brief Replaces all occurrences of a specified variable in a SPARQL query string.
 *
 * @param query The SPARQL query string in which the variable replacement will occur.
 * @param from The variable name to be replaced in the query string.
 * @param to The new value that will replace the specified variable in the query string.
 */
void TripleAssembler::replaceAllSparqlVariables(std::string& query, const std::string& from,
                                                const std::string& to) {
    size_t start_pos = 0;
    while ((start_pos = query.find(from, start_pos)) != std::string::npos) {
        query.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

/**
 * @brief Extracts subject, predicate, and object values from a query string.
 *
 * @param query The input query string from which to extract values.
 * @return A tuple containing the subject, predicate, and object as strings.
 */
std::tuple<std::string, std::string, std::string> TripleAssembler::extractElementValuesFromQuery(
    const std::string& query) {
    std::istringstream stream(query);
    std::string line;

    // Skip the first line
    std::getline(stream, line);

    // Read the second line
    std::getline(stream, line);

    // Extract the three values
    std::istringstream second_line_stream(line);
    std::string subject, predicate, object;
    second_line_stream >> subject >> predicate >> object;

    return std::make_tuple(subject, predicate, object);
}

/**
 * @brief Extracts prefix declarations from a SPARQL query string.
 *
 * @param query The input SPARQL query string from which to extract prefix declarations.
 * @return A string containing all prefix declarations found in the query, each on a new line.
 */
std::string TripleAssembler::extractPrefixesFromQuery(const std::string& query) {
    // Regular expression to match lines that start with "prefix" and capture the entire line
    std::regex prefix_regex(R"((prefix\s+\w+:\s+<[^>]+>))", std::regex::icase);
    std::smatch match;
    std::string result;
    std::istringstream stream(query);
    std::string line;

    // Iterate through each line and check if it matches the prefix pattern
    while (std::getline(stream, line)) {
        if (std::regex_search(line, match, prefix_regex)) {
            result += line + "\n";
        }
    }

    return result;
}

/**
 * @brief Stores the triple output to a file.
 *
 * This function constructs a file name using the configured output file path,
 * the current timestamp, and the appropriate file extension. It then writes
 * the provided triple output to this file.
 *
 * @param triple_output The string containing the triple output to be stored.
 */
void TripleAssembler::storeTripleOutput(const std::string& triple_output) {
    if (!rdfox_adapter_.loadData(triple_output)) {
        std::cerr << "It was a problem loading triple data to RDF-Server" << std::endl;
    }

    // create file name
    const std::string file_name = model_config_.output_file_path + "gen_rdf_triple_" +
                                  Helper::getFormattedTimestamp("%H", false, true) +
                                  getFileExtension();

    // Add the current time to the log
    std::ostringstream output;
    output << "# Output from " << Helper::getFormattedTimestamp("%Y-%m-%dT%H:%M:%S", true, true)
           << "\n\n"
           << triple_output << "\n\n";

    // Write the file
    file_handler_.writeFile(file_name, output.str(), true);
    std::cout << "A triple has been generated under: " << file_name << std::endl;
}

/**
 * @brief Determines the file extension based on the configured RDF output format.
 *
 * This function retrieves the appropriate file extension for the RDF output format
 * specified in the model configuration. It supports various RDF serialization formats
 * such as Turtle, N-Quads, N-Triples, and TriG.
 *
 * @return A string representing the file extension for the current RDF syntax type.
 * @throws std::runtime_error If the RDF syntax type is unsupported.
 */
std::string TripleAssembler::getFileExtension() {
    switch (model_config_.reasoner_settings.output_format) {
        case RDFSyntaxType::TURTLE:
            return ".ttl";
        case RDFSyntaxType::NQUADS:
            return ".nq";
        case RDFSyntaxType::NTRIPLES:
            return ".nt";
        case RDFSyntaxType::TRIG:
            return ".trig";
        default:
            throw std::runtime_error("Unsupported Serd syntax format");
    }
}