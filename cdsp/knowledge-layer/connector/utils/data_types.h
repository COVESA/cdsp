#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "helper.h"

/**
 * @brief Enum class for the message types
 */
enum class MessageType {
    DATA,
    GET,
    SET,
    SUBSCRIBE,
    UNSUBSCRIBE,
};

/**
 * @brief Enum class for the supported schema types
 */
enum class SchemaType {
    VEHICLE,
    DEFAULT,
};

/**
 * @brief Enum class for the RDF syntax types
 */
enum class RDFSyntaxType {
    TURTLE,    ///< Terse triples http://www.w3.org/TR/turtle
    NTRIPLES,  ///< Line-based triples http://www.w3.org/TR/n-triples/
    NQUADS,    ///< Line-based quads http://www.w3.org/TR/n-quads/
    TRIG,      ///< Terse quads http://www.w3.org/TR/trig/
};

/**
 * @brief Enum class for the data query accept types
 */
enum class DataQueryAcceptType {
    TEXT_TSV,
    TEXT_CSV,
    SPARQL_JSON,
    SPARQL_XML,
};

/**
 * @brief Enum class for the supported message structure formats
 */
enum class MessageStructureFormat {
    FLAT,
    NESTED,
    LEAF,
};

/**
 * @brief Configuration structure for the reasoner settings
 */
struct ReasonerSettings {
    std::string inference_engine;
    RDFSyntaxType output_format;
    std::vector<SchemaType> supported_schema_collections;
};

/**
 * @brief Configuration structure for the use-case model
 */
struct ModelConfig {
    std::map<SchemaType, std::vector<std::string>> system_data_points;
    std::string output_file_path;
    std::vector<std::string> ontology_files;
    std::vector<std::string> shacl_shapes_files;
    std::map<SchemaType, std::vector<std::string>> triple_assembler_queries_files;
    std::string output_queries_path;
    std::vector<std::string> rules_files;
    ReasonerSettings reasoner_settings;
};

/**
 * @brief Configuration structure for the servers
 */
struct ServerData {
    std::string host;
    std::string port;
    std::string auth_base64;
    std::optional<std::string> data_store;
};

/**
 * @brief Configuration structure for the WebSocket client
 */
struct InitConfig {
    std::string uuid;
    ServerData websocket_server;
    ServerData rdfox_server;
    std::map<SchemaType, std::string> oid;
    ModelConfig model_config;
};

/**
 * @brief InternalErrorMessage structure for the error message
 */

struct InternalErrorMessage {
    std::string type;
    int errorCode;
    std::string message;
};

/**
 * @brief Converts a MessageType enum to its corresponding string representation.
 *
 * @param type The MessageType enum value to be converted.
 * @return A std::string representing the name of the MessageType.
 *
 * @throws std::runtime_error if the input MessageType is not supported.
 */
std::string messageTypeToString(const MessageType& type);

/**
 * @brief Converts a reasoner output format string to an RDFSyntaxType.
 *
 * This function takes a string representing a reasoner output format and
 * converts it to the corresponding RDFSyntaxType. The input string should
 * match one of the predefined reasoner output formats.
 *
 * @param type A string representing the reasoner output format.
 * @return The corresponding RDFSyntaxType for the given reasoner output format.
 *
 * @throws std::runtime_error if the input reasoner output format is not supported.
 */
RDFSyntaxType reasonerOutputFormatToRDFSyntaxType(const std::string& type);

/**
 * @brief Converts an RDF syntax type to its corresponding content type string.
 *
 * This function takes an RDFSyntaxType enumeration value and returns the
 * corresponding content type for the REST API as a string. This is useful for determining
 * the MIME type associated with a particular RDF syntax.
 *
 * @param type The RDF syntax type to be converted.
 * @return A string representing the content type associated with the given RDF syntax type.
 */
std::string RDFSyntaxTypeToContentType(const RDFSyntaxType& type);

/**
 * @brief Converts a QueryAcceptType enum to its corresponding string representation.
 *
 * @param type The DataQueryAcceptType enum value to be converted.
 * @return A std::string representing the name of the DataQueryAcceptType.
 *
 * @throws std::runtime_error if the input DataQueryAcceptType is not supported.
 */
std::string queryAcceptTypeToString(const DataQueryAcceptType& type);

/**
 * @brief Converts a SchemaType enum value to its corresponding string representation.
 *
 * This function takes a SchemaType enum value as input and returns a string that
 * represents the name of the schema type.
 *
 * @param type The SchemaType enum value to be converted to a string.
 * @param capitalizeFirstLetter Boolean flag to capitalize the first letter of the string if true.
 * @return A std::string representing the name of the schema type.
 *
 * @throws std::runtime_error if the input SchemaType is not supported.
 */
std::string SchemaTypeToString(const SchemaType& type, bool capitalizeFirstLetter = false);

/**
 * @brief Converts a string to a SchemaType enum value.
 *
 * This function takes a string as input and converts it to the corresponding
 * SchemaType enum value. The input string should match one of the predefined
 * schema types.
 *
 * @param type A string representing the schema type.
 * @return The corresponding SchemaType enum value for the given string.
 *
 * @throws std::runtime_error if the input string does not match any of the predefined schema
 * definitions.
 */
SchemaType stringToSchemaType(const std::string& type);

/**
 * @brief Converts a structure message format enum to its corresponding string representation.
 *
 * @param type The MessageStructureFormat enum value to be converted.
 * @return A std::string representing the name of the MessageStructureFormat.
 *
 * @throws std::runtime_error if the input MessageStructureFormat is not supported.
 */
std::string stringToMessageStructureFormat(const MessageStructureFormat& type);

inline std::string messageTypeToString(const MessageType& type) {
    switch (type) {
        case MessageType::DATA:
            return "data";
        case MessageType::GET:
            return "get";
        case MessageType::SET:
            return "set";
        case MessageType::SUBSCRIBE:
            return "subscribe";
        case MessageType::UNSUBSCRIBE:
            return "unsubscribe";
        default:
            throw std::runtime_error("Unsupported message type");
    }
};

inline RDFSyntaxType reasonerOutputFormatToRDFSyntaxType(const std::string& type) {
    std::string lowerCaseType = Helper::toLowerCase(type);
    if (lowerCaseType == "turtle") {
        return RDFSyntaxType::TURTLE;
    } else if (lowerCaseType == "ntriples") {
        return RDFSyntaxType::NTRIPLES;
    } else if (lowerCaseType == "nquads") {
        return RDFSyntaxType::NQUADS;
    } else if (lowerCaseType == "trig") {
        return RDFSyntaxType::TRIG;
    } else {
        throw std::invalid_argument("Unsupported RDF output format");
    }
}

inline std::string RDFSyntaxTypeToContentType(const RDFSyntaxType& type) {
    switch (type) {
        case RDFSyntaxType::TURTLE:
            return "text/turtle";
        case RDFSyntaxType::NTRIPLES:
            return "application/n-triples";
        case RDFSyntaxType::NQUADS:
            return "application/n-quads";
        case RDFSyntaxType::TRIG:
            return "application/trig";
        default:
            throw std::invalid_argument("Unsupported RDF syntax type");
    }
}

inline std::string queryAcceptTypeToString(const DataQueryAcceptType& type) {
    switch (type) {
        case DataQueryAcceptType::TEXT_CSV:
            return "text/csv";
        case DataQueryAcceptType::TEXT_TSV:
            return "text/tab-separated-values";
        case DataQueryAcceptType::SPARQL_JSON:
            return "application/sparql-results+json";
        case DataQueryAcceptType::SPARQL_XML:
            return "application/sparql-results+xml";
        default:
            throw std::invalid_argument("Unsupported query accept type");
    }
};

inline std::string SchemaTypeToString(const SchemaType& type, bool capitalizeFirstLetter) {
    std::string schema;
    switch (type) {
        case SchemaType::VEHICLE:
            schema = "vehicle";
            break;
        default:
            throw std::invalid_argument("Unsupported schema type");
    }
    if (capitalizeFirstLetter) {
        schema[0] = toupper(schema[0]);
    }
    return schema;
};

inline SchemaType stringToSchemaType(const std::string& type) {
    std::string lowerCaseType = Helper::toLowerCase(type);
    if (lowerCaseType == "vehicle") {
        return SchemaType::VEHICLE;
    } else {
        throw std::invalid_argument("Unsupported schema type");
    }
}

inline std::string stringToMessageStructureFormat(const MessageStructureFormat& type) {
    switch (type) {
        case MessageStructureFormat::FLAT:
            return "flat";
        case MessageStructureFormat::NESTED:
            return "nested";
        case MessageStructureFormat::LEAF:
            return "leaf";
        default:
            throw std::invalid_argument("Unsupported message structure format");
    }
}

#endif  // DATA_TYPES_H