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

enum class RDFSyntaxType {
    TURTLE,    ///< Terse triples http://www.w3.org/TR/turtle
    NTRIPLES,  ///< Line-based triples http://www.w3.org/TR/n-triples/
    NQUADS,    ///< Line-based quads http://www.w3.org/TR/n-quads/
    TRIG,      ///< Terse quads http://www.w3.org/TR/trig/
};

struct ReasonerSettings {
    std::string inference_engine;
    RDFSyntaxType output_format;
    std::vector<std::string> supported_tree_types;
};

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

struct ServerData {
    std::string host;
    std::string port;
    std::string auth_base64;
    std::optional<std::string> data_store;
};

struct InitConfig {
    std::string uuid;
    ServerData websocket_server;
    ServerData rdfox_server;
    std::string oid;
    ModelConfig model_config;
};

struct MessageHeader {
    std::string id;
    std::string type;
    std::string tree;
    std::string date_time;
    std::string uuid;
};

struct Node {
    std::string name;
    std::string value;
};

struct DataMessage {
    MessageHeader header;
    std::vector<Node> nodes;
};

struct ErrorNode {
    std::string name;
    std::string status;
};

struct ErrorMessage {
    std::string type;
    std::variant<std::string, std::vector<ErrorNode>> error;
    int errorCode;
};

struct CategoryMessage {
    std::string category;
    std::string message;
    int statusCode;
};

enum class MessageType {
    READ,
    WRITE,
    SUBSCRIBE,
    UNSUBSCRIBE,
};

std::string messageTypeToString(const MessageType& type);

RDFSyntaxType reasonerOutputFormatToRDFSyntaxType(const std::string& type);

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

#endif  // DATA_TYPES_H