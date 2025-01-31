#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

// Forward declare the Helper namespace functions
namespace Helper {
std::string toLowerCase(const std::string& input);
}

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
 * @brief Configuration structure for the reasoner settings
 */
struct ReasonerSettings {
    std::string inference_engine;
    RDFSyntaxType output_format;
    std::vector<std::string> supported_tree_types;
};

/**
 * @brief Configuration structure for the use-case model
 */
struct ModelConfig {
    std::map<std::string, std::vector<std::string>> system_data_points;
    std::string output_file_path;
    std::vector<std::string> ontology_files;
    std::vector<std::string> shacl_shapes_files;
    std::map<std::string, std::vector<std::string>> triple_assembler_queries_files;
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
    std::string oid;
    ModelConfig model_config;
};

/**
 * @brief Header structure for the DataMessage
 */
struct MessageHeader {
    std::string id;
    std::string type;
    std::string tree;
    std::string date_time;
    std::string uuid;
    std::string schema;
};

/**
 * @brief Metadata structure for a Node
 */
struct Metadata {
    struct Timestamp {
        int64_t seconds;
        int32_t nanos;
    };
    Timestamp received;
};

/**
 * @brief Represents a Node in the business object
 */
struct Node {
    std::string name;
    std::string value;
    Metadata metadata;
};

/**
 * @brief DataMessage structure containing a header and a list of nodes
 */
struct DataMessage {
    MessageHeader header;
    std::vector<Node> nodes;
};

/**
 * @brief ErrorNode structure for the error message
 */
struct ErrorNode {
    std::string name;
    std::string status;
};

/**
 * @brief ErrorMessage structure for the error message
 */
struct ErrorMessage {
    std::string type;
    std::variant<std::string, std::vector<ErrorNode>> error;
    int errorCode;
};

/**
 * @brief CategoryMessage structure for the category message
 */
struct CategoryMessage {
    std::string category;
    std::string message;
    int statusCode;
};

/**
 * @brief Enum class for the message types
 */
enum class MessageType {
    READ,
    WRITE,
    SUBSCRIBE,
    UNSUBSCRIBE,
};

/**
 * @brief Converts a MessageType enum to its corresponding string representation.
 *
 * @param type The MessageType enum value to be converted.
 * @return A std::string representing the name of the MessageType.
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
 */
RDFSyntaxType reasonerOutputFormatToRDFSyntaxType(const std::string& type);

/**
 * @brief Converts a QueryAcceptType enum to its corresponding string representation.
 *
 * @param type The DataQueryAcceptType enum value to be converted.
 * @return A std::string representing the name of the DataQueryAcceptType.
 */
std::string queryAcceptTypeToString(const DataQueryAcceptType& type);

inline std::string messageTypeToString(const MessageType& type) {
    switch (type) {
        case MessageType::READ:
            return "read";
        case MessageType::WRITE:
            return "write";
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
        throw std::runtime_error("Unsupported RDF output format");
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
            throw std::runtime_error("Unsupported query accept type");
    }
};

#endif  // DATA_TYPES_H