#ifndef DATA_TYPES_DATA_TYPES_H
#define DATA_TYPES_DATA_TYPES_H

#include <chrono>
#include <map>
#include <string>
#include <vector>

struct ReasonerSettings {
    std::string inference_engine;
    std::string output_format;
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

struct InitConfig {
    std::string uuid;
    std::string host_websocket_server;
    std::string port_websocket_server;
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

enum class MessageType {
    READ,
    WRITE,
    SUBSCRIBE,
    UNSUBSCRIBE,
};

inline std::string messageTypeToString(MessageType type) {
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
}

#endif  // DATA_TYPES_DATA_TYPES_H