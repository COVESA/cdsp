#include "triple_assembler.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>

#include "data_message.h"
#include "helper.h"

using json = nlohmann::json;

TripleAssembler::TripleAssembler(std::shared_ptr<ModelConfig> model_config,
                                 ReasonerService& reasoner_service, IFileHandler& file_reader,
                                 TripleWriter& triple_writer)
    : model_config_(model_config),
      reasoner_service_(reasoner_service),
      file_handler_(file_reader),
      triple_writer_(triple_writer) {}

/**
 * @brief Initializes the TripleAssembler by checking the data store and loading validation shapes.
 *
 * This function first checks if the data store is available using the reasoner service. If the data
 * store is not available, it throws a runtime error. It then attempts to load validation shapes
 * from the model configuration. If no validation shapes are found or if loading fails, it throws a
 * runtime error.
 *
 * @throws std::runtime_error If the data store is unavailable or if validation shapes cannot be
 * loaded.
 */
void TripleAssembler::initialize() {
    if (!reasoner_service_.checkDataStore()) {
        throw std::runtime_error("Initialization failed: Unable to generate triples.");
    }
    const auto validation_shapes = model_config_->getValidationShapes();
    if (!validation_shapes.empty()) {
        for (const auto& [reasoner_syntax_type, data] : validation_shapes) {
            if (data.empty() || !reasoner_service_.loadData(data, reasoner_syntax_type)) {
                throw std::runtime_error(
                    "No validation shapes could be loaded. The triples cannot be generated.");
            }
        }
    } else {
        throw std::runtime_error(
            "No validation shapes were found to load. The triples cannot be generated.");
    }
}

/**
 * Transforms a DataMessage into reasoning triples and stores the output.
 *
 * This function processes a given DataMessage by extracting its header and nodes,
 * and then generates reasoning triples based on the nodes' data. It checks the data store
 * availability before proceeding and handles both coordinate and non-coordinate nodes
 * differently. If valid coordinates are found, it generates triples specifically for them.
 * Finally, it outputs the generated triples in the specified format.
 *
 * @param message The DataMessage containing the header and nodes to be transformed into triples.
 * @throws std::runtime_error If the data store check fails.
 */
void TripleAssembler::transformMessageToTriple(const DataMessage& message) {
    if (!reasoner_service_.checkDataStore()) {
        throw std::runtime_error("Failed to call datastore. The triples cannot be generated.");
    }

    MessageHeader header = message.getHeader();
    std::vector<Node> nodes = message.getNodes();

    // Add the identifier to the triples that will be generated
    triple_writer_.initiateTriple(header.getId());

    if (nodes.empty()) {
        std::cout << "No nodes found in the message\n\n";
        return;
    }

    std::optional<CoordinateNodes> valid_coordinates = std::nullopt;

    for (const auto node : nodes) {
        if (node.getName() == "Vehicle.CurrentLocation.Latitude" ||
            node.getName() == "Vehicle.CurrentLocation.Longitude") {
            const auto node_timestamp = getTimestampFromNode(node);
            const auto nanoseconds_since_epoch = Helper::getNanosecondsSinceEpoch(node_timestamp);

            const auto found_key =
                timestamp_coordinates_messages_map_.find(nanoseconds_since_epoch);
            if (found_key != timestamp_coordinates_messages_map_.end()) {
                found_key->second.insert({node.getName(), node});
            } else {
                timestamp_coordinates_messages_map_.emplace(
                    nanoseconds_since_epoch,
                    std::unordered_map<std::string, Node>{{node.getName(), node}});
            }

            valid_coordinates = getValidCoordinatesPair();

        } else {
            try {
                generateTriplesFromNode(node, header.getSchemaType());
            } catch (const std::exception& e) {
                std::cerr << "An error occurred creating the triples: " << e.what() << std::endl;
            }
        }
    }

    if (valid_coordinates.has_value())
        generateTriplesFromCoordinates(valid_coordinates, header.getSchemaType(), message);

    // Get the document of the generated triples
    std::string generated_triples =
        triple_writer_.generateTripleOutput(model_config_->getReasonerSettings().getOutputFormat());

    if (!generated_triples.empty()) {
        storeTripleOutput(generated_triples);
    } else {
        std::cout << "No triples have been generated for the update message\n\n";
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
    chrono_time_nanos last_timestamp_to_delete(0);
    const Node* latitude = nullptr;
    chrono_time_nanos latitude_time;
    const Node* longitude = nullptr;
    chrono_time_nanos longitude_time;

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
        const auto two_seconds_in_nanoseconds = std::chrono::nanoseconds(2'000'000'000);
        auto difference = latitude_time - longitude_time;

        if (std::abs(difference.count()) <= two_seconds_in_nanoseconds.count()) {
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
 * @brief Generates reasoning triples from a given node.
 *
 * This function processes a node by extracting its object and data elements,
 * querying necessary prefixes and values, and then adding these as objects
 * and data to the triple writer. It handles specific node names
 * related to vehicle location by preparing additional data if necessary.
 *
 * @param node The node containing the name and value to be transformed into reasoning triples.
 * @param msg_schema_type The message schema type used for querying data.
 * @param node_timestamp The timestamp associated with the node.
 * @param ntm_coord_value An optional coordinate value for NTM data.
 */
void TripleAssembler::generateTriplesFromNode(const Node& node, const SchemaType& msg_schema_type,
                                              const std::optional<double>& ntm_coord_value) {
    try {
        // Split node data point into object and data elements
        const auto [object_elements, data_element] = extractObjectsAndDataElements(node.getName());

        const auto queries = model_config_->getQueriesTripleAssemblerHelper().getQueries();
        TripleAssemblerHelper::QueryPair query_pair;
        if (queries.find(msg_schema_type) != queries.end()) {
            query_pair = queries.at(msg_schema_type);
        } else {
            query_pair = queries.at(SchemaType::DEFAULT);
        }

        // Query and add Object Elements
        for (std::size_t i = 1; i < object_elements.size(); ++i) {
            const auto [prefixes, object_values] = getQueryPrefixesAndData(
                query_pair.object_property, object_elements[i - 1], object_elements[i]);

            triple_writer_.addElementObjectToTriple(prefixes, object_values);
        }

        // Query and add Data Element
        const auto [prefixes, data_values] = getQueryPrefixesAndData(
            query_pair.data_property, object_elements[object_elements.size() - 1], data_element);

        const auto node_timestamp = getTimestampFromNode(node);

        triple_writer_.addElementDataToTriple(prefixes, data_values, node.getValue().value(),
                                              node_timestamp, ntm_coord_value);
    } catch (const std::exception& e) {
        std::cerr << "An error occurred while creating the reasoning triples: " << e.what()
                  << std::endl;
        throw;
    }
}

/**
 * @brief Retrieves a valid pair of latitude and longitude coordinates.
 *
 * This function iterates through the `timestamp_coordinates_messages_map_` to find
 * the most recent latitude and longitude nodes. It checks if both coordinates exist
 * and ensures they are recorded within a 2-second interval. If valid, it updates
 * the `coordinates_last_time_stamp_` and returns the coordinate nodes.
 *
 * @param valid_coordinates The optional containing the latitude and longitude nodes.
 * @param msg_schema_type The message schema type used for querying data.
 * @param message The DataMessage containing nodes and metadata to be transformed into triples.
 *
 * @return An optional CoordinateNodes object containing the latitude and longitude nodes
 * if both are found and valid; otherwise, std::nullopt.
 */
void TripleAssembler::generateTriplesFromCoordinates(
    std::optional<CoordinateNodes>& valid_coordinates, const SchemaType& msg_schema_type,
    const DataMessage& message) {
    {
        try {
            auto ntm_coord =
                Helper::getCoordInNtm(valid_coordinates.value().latitude.getValue().value(),
                                      valid_coordinates.value().longitude.getValue().value());
            if (ntm_coord == std::nullopt) {
                throw std::runtime_error("Failed to convert coordinates to NTM");
            }

            generateTriplesFromNode(valid_coordinates.value().latitude, msg_schema_type,
                                    ntm_coord.value().northing);
            generateTriplesFromNode(valid_coordinates.value().longitude, msg_schema_type,
                                    ntm_coord.value().easting);
        } catch (const std::exception& e) {
            std::cerr << "An error occurred creating the TTL triples: " << e.what() << std::endl;
        }

        // Manual cleanup of old timestamps
        cleanupOldTimestamps();
    }
}

/**
 * Retrieves the timestamp from a given node.
 *
 * This function extracts the timestamp from the node's metadata. It first checks if the
 * 'generated' timestamp is available and returns it if present. If the 'generated' timestamp
 * is not available, it falls back to returning the 'received' timestamp.
 *
 * @param node The node from which to retrieve the timestamp.
 * @return The timestamp as a std::chrono::system_clock::time_point.
 */
const std::chrono::system_clock::time_point TripleAssembler::getTimestampFromNode(
    const Node& node) {
    std::chrono::system_clock::time_point node_timestamp;

    if (node.getMetadata().getGenerated().has_value()) {
        node_timestamp = node.getMetadata().getGenerated().value();
    } else {
        node_timestamp = node.getMetadata().getReceived();
    }
    return node_timestamp;
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
    std::vector<std::string> elements = Helper::splitString(node_name, '.');

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
 * Retrieves prefixes and data values from a given query.
 *
 * This function processes a query by replacing placeholders with the provided
 * schema types and object classes. It then queries the reasoner service to retrieve
 * the prefixes and data values from the formatted query.
 *
 * @param query The query to be processed and language type.
 * @param subject_class The subject class to be used in the query.
 * @param object_class The object class to be used in the query.
 * @return A pair where the first element is a string representing the prefixes,
 *         and the second element is a tuple containing the subject, predicate, and object values.
 * @throws std::runtime_error if no data is returned for the formatted query.
 */
std::pair<std::string, std::tuple<std::string, std::string, std::string>>
TripleAssembler::getQueryPrefixesAndData(const std::pair<QueryLanguageType, std::string>& query,
                                         const std::string& subject_class,
                                         const std::string& object_class) {
    std::string formattedQuery = query.second;

    replaceAllQueryVariables(formattedQuery, "%A%", subject_class);
    replaceAllQueryVariables(formattedQuery, "%B%", object_class);

    const std::string query_result = reasoner_service_.queryData(formattedQuery, query.first);
    if (query_result.empty()) {
        throw std::runtime_error("No data returned for the formatted query.");
    }

    const auto element_values = extractElementValuesFromQuery(query_result);
    const std::string prefixes = extractPrefixesFromQuery(formattedQuery);
    return std::make_pair(prefixes, element_values);
}

/**
 * @brief Replaces all occurrences of a substring in a query string.
 *
 * This function replaces all occurrences of a substring in a query string with a new substring.
 * It iterates through the query string and replaces all instances of the 'from' substring with
 * the 'to' substring.
 */
void TripleAssembler::replaceAllQueryVariables(std::string& query, const std::string& from,
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
 * @brief Extracts prefix declarations from a query string.
 *
 * @param query The input query string from which to extract prefix declarations.
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
    const ReasonerSyntaxType output_format = model_config_->getReasonerSettings().getOutputFormat();
    if (!reasoner_service_.loadData(triple_output, output_format)) {
        std::cerr << "It was a problem loading triple data to Reasoner-Server" << std::endl;
    }

    // Create file name
    const std::string file_name = model_config_->getOutput() + "triples/gen_triple_t_" +
                                  Helper::getFormattedTimestampNow("%H", false, true) +
                                  reasonerSyntaxTypeToFileExtension(output_format);

    // Add the current time to the log
    std::ostringstream output;
    output << "# Output from " << Helper::getFormattedTimestampNow("%Y-%m-%dT%H:%M:%S", true, true)
           << "\n\n"
           << triple_output << "\n\n";

    // Write the file
    file_handler_.writeFile(file_name, output.str(), true);
    std::cout << "A triple has been generated under: " << file_name << std::endl << std::endl;
}
