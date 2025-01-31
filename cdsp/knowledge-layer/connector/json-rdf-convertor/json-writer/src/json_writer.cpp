#include "json_writer.h"

#include <iostream>
#include <sstream>

#include "file_handler_impl.h"
#include "helper.h"
#include "pugixml.hpp"

/**
 * Writes the query result to a JSON object and optionally stores it in a file.
 *
 * This function takes a string representing the result of a query, the format of the result,
 * and an optional output file path. It converts the query result into a JSON object and stores
 * it in a file if an output file path is provided. The JSON object contains the query result
 * data in a structured format, along with a timestamp indicating when the object was created.
 *
 * @param query_result A string containing the result of a query.
 * @param result_format_type The format of the query result.
 * @param output_file_path An optional file path to store the JSON object.
 * @param file_handler An optional file handler to use for writing the JSON object to a file.
 * @return A JSON object containing the query result data and metadata.
 * @throws std::runtime_error If the query result format is not supported.
 */
nlohmann::json JSONWriter::writeToJson(const std::string& query_result,
                                       const DataQueryAcceptType& result_format_type,
                                       std::optional<std::string> output_file_path,
                                       std::shared_ptr<IFileHandler> file_handler) {
    nlohmann::json json_result;
    if (result_format_type == DataQueryAcceptType::TEXT_CSV) {
        json_result = parseTableFormat(query_result, ',');
    } else if (result_format_type == DataQueryAcceptType::TEXT_TSV) {
        json_result = parseTableFormat(query_result, '\t');
    } else if (result_format_type == DataQueryAcceptType::SPARQL_JSON) {
        json_result = parseSparqlJson(query_result);
    } else if (result_format_type == DataQueryAcceptType::SPARQL_XML) {
        json_result = parseSparqlXml(query_result);
    } else {
        throw std::runtime_error("Unsupported query result format");
    }
    nlohmann::json json_object;
    json_object["created_at"] = Helper::getFormattedTimestamp("%Y-%m-%dT%H:%M:%S", true, true);
    json_object["results"] = json_result;

    !output_file_path.value_or("").empty()
        ? storeJsonToFile(json_object, *output_file_path, file_handler)
        : void();
    return json_object;
}

/**
 * Parses a delimited table format string and converts it into a JSON array string.
 *
 * This function takes a string representing a table with a specified delimiter,
 * parses it, and converts it into a JSON array format. Each row in the table is
 * represented as a JSON object within the array, where the keys are the column
 * headers from the first row of the table and the values are the corresponding
 * cell values. Underscores in header names are replaced with dots.
 *
 * @param query_result A string containing the table data to be parsed.
 * @param delimiter A character used to separate values in the table.
 * @return A string representing the JSON array of the parsed table rows, formatted with an
 * indentation of 2 spaces.
 */
nlohmann::json JSONWriter::parseTableFormat(const std::string& query_result, char delimiter) {
    std::stringstream ss(query_result);
    std::string line;
    std::vector<std::string> headers;
    nlohmann::json json_array = nlohmann::json::array();

    // Read headers from the first line
    if (std::getline(ss, line)) {
        std::stringstream header_stream(line);
        std::string header;
        while (std::getline(header_stream, header, delimiter)) {
            header.erase(std::remove(header.begin(), header.end(), '\r'), header.end());
            std::replace(header.begin(), header.end(), '_', '.');  // Convert _ to .
            headers.push_back(header);
        }
    }

    // Process each row
    while (std::getline(ss, line)) {
        std::stringstream row_stream(line);
        std::string value;
        nlohmann::json row_object;
        size_t col_idx = 0;

        while (std::getline(row_stream, value, delimiter)) {
            value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
            if (col_idx < headers.size()) {
                row_object[headers[col_idx]] = Helper::detectType(value);
            }
            col_idx++;
        }

        // Ensure all expected headers exist in the row
        for (const auto& header : headers) {
            if (!row_object.contains(header)) {
                row_object[header] = "";
            }
        }

        json_array.push_back(row_object);
    }

    return json_array;
}

/**
 * Parses a SPARQL JSON result and converts it into a JSON array string.
 *
 * This function takes a JSON string representing the result of a SPARQL query,
 * parses it, and converts it into a JSON array format. Each SPARQL result is
 * represented as a JSON object within the array, where the keys are the variable
 * names from the SPARQL query and the values are the corresponding result values.
 * Underscores in variable names are replaced with dots.
 *
 * @param json_result A string containing the SPARQL JSON result to be parsed.
 * @return A string representing the JSON array of the parsed SPARQL results, formatted with an
 * indentation of 2 spaces.
 * @throws std::runtime_error If the JSON string does not contain the expected SPARQL result format.
 */
nlohmann::json JSONWriter::parseSparqlJson(const std::string& json_result) {
    nlohmann::json sparql_json = nlohmann::json::parse(json_result);
    nlohmann::json json_array = nlohmann::json::array();

    if (!sparql_json.contains("results") || !sparql_json["results"].contains("bindings")) {
        throw std::runtime_error("Invalid SPARQL JSON response format");
    }

    for (const auto& binding : sparql_json["results"]["bindings"]) {
        nlohmann::json row_object;

        for (auto it = binding.begin(); it != binding.end(); ++it) {
            std::string var_name = it.key();
            std::string value = it.value()["value"].get<std::string>();

            std::replace(var_name.begin(), var_name.end(), '_', '.');

            row_object[var_name] = Helper::detectType(value);
        }

        json_array.push_back(row_object);
    }

    return json_array;
}

/**
 * Parses a SPARQL XML result and converts it into a JSON array string.
 *
 * This function takes an XML string representing the result of a SPARQL query,
 * parses it, and converts it into a JSON array format. Each SPARQL result is
 * represented as a JSON object within the array, where the keys are the variable
 * names from the SPARQL query and the values are the corresponding result values.
 * Underscores in variable names are replaced with dots.
 *
 * @param xml_result A string containing the SPARQL XML result to be parsed.
 * @return A string representing the JSON array of the parsed SPARQL results, formatted with an
 * indentation of 2 spaces.
 * @throws std::runtime_error If the XML string cannot be parsed.
 */
nlohmann::json JSONWriter::parseSparqlXml(const std::string& xml_result) {
    pugi::xml_document doc;
    if (!doc.load_string(xml_result.c_str())) {
        throw std::runtime_error("Failed to parse SPARQL XML response");
    }

    nlohmann::json json_array = nlohmann::json::array();

    // Navigate to <results> node
    pugi::xml_node results_node = doc.child("sparql").child("results");

    for (pugi::xml_node result : results_node.children("result")) {
        nlohmann::json row_object;

        for (pugi::xml_node binding_node : result.children("binding")) {
            std::string name = binding_node.attribute("name").value();
            std::string value;

            // Handle any value type (literal, URI, etc.)
            for (pugi::xml_node child : binding_node.children()) {
                value = child.child_value();
                break;
            }
            std::replace(name.begin(), name.end(), '_', '.');
            row_object[name] = Helper::detectType(value);
        }
        json_array.push_back(row_object);
    }

    return json_array;
}

/**
 * @brief Stores a JSON object to a file at a specified path.
 *
 * This function ensures that the directory for the output file path exists,
 * creates a file name with a timestamp, and writes the JSON data to the file.
 * If a file handler is not provided, a default implementation is used.
 *
 * @param json_data The JSON object to be stored in the file.
 * @param output_file_path The path to the directory where the JSON file will be stored.
 *                         The function will create the directory if it does not exist.
 * @param file_handler An optional shared pointer to an IFileHandler implementation.
 *                     If not provided, a default FileHandlerImpl is used.
 */
void JSONWriter::storeJsonToFile(const nlohmann::json& json_data,
                                 const std::string& output_file_path,
                                 std::shared_ptr<IFileHandler> file_handler) {
    namespace fs = std::filesystem;

    // Ensure the directory exists
    if (!fs::exists(output_file_path)) {
        std::cout << "Creating directory: " << output_file_path << std::endl;
        fs::create_directories(output_file_path);
    }

    // Create file name
    const std::string file_name = output_file_path + "gen_from_sparql_query_" +
                                  Helper::getFormattedTimestamp("%H", false, true) + ".json";

    // Write the file
    if (!file_handler) {
        file_handler = std::make_shared<FileHandlerImpl>();
    }
    file_handler->writeFile(file_name, json_data.dump(2), false);
    std::cout << "A JSON SPARQL Output has been generated under: " << file_name << std::endl;
}