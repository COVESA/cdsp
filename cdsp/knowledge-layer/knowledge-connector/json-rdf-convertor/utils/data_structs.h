///
/// @file
/// @copyright Copyright (C) 2017-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///

#ifndef DATA_STRUCTS_DATA_STRUCTS_H
#define DATA_STRUCTS_DATA_STRUCTS_H

#include <chrono>
#include <string>
#include <vector>

struct MessageHeader
{
    std::string id;
    std::string type;
    std::string tree;
    std::string date_time;
    std::string timestamp;
    std::chrono::duration<double, std::milli> time_stamp;
};

struct Node
{
    std::string name;
    std::string date_time;
    std::string timestamp;
    std::vector<std::string> values;
    std::chrono::duration<double, std::milli> time_stamp;
};

struct Data
{
    MessageHeader header;
    std::vector<Node> nodes;
};

struct DataPoint
{
    std::string name;
    std::string value;
};

struct InferredResult
{
    std::string entity_id;
    std::vector<DataPoint> data;
};

struct ProgramOptions
{
    std::string map;
    std::string output_ttl;
    std::string persist_data;
    std::string entity_id;

    bool enable_message_log;
    bool test_rules;
    bool dummy_server;
};

struct QueryData
{
    std::string name_one;
    std::string name_two;
    std::string has_statement;
    std::string data_type;
    std::string prefix_name_one;
    std::string prefix_name_two;
    std::string prefix_data_type;
    std::string prefix_has_statement;
};

#endif  // DATA_STRUCTS_DATA_STRUCTS_H
