///
/// @file
/// @copyright Copyright (C) 2017-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///

#ifndef UTILS_UTILS_H
#define UTILS_UTILS_H

#include <string.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <unordered_map>
#include <vector>

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>

#include "data_structs.h"

namespace utils
{
namespace parser
{
Data GetVehicleFeatureDataFromString(const std::string& string_feature);

void RemoveChars(const char chars_to_remove[], std::string& string);

void RemoveSubstring(const std::string& to_erase, std::string& string_feature);

void SubstituteCharacter(std::string& input, char a, char b);

Json::Value ParseJsonFromStream(const std::string& input_json);

Json::Value ParseJsonFromString(const std::string& input_json);

std::string FindDataTypesInJsonStream(const Json::Value& json_value,
                                      const std::string& name_structure,
                                      std::unordered_map<std::string, std::string>& data_types_map);

std::chrono::duration<double, std::milli> GetChronoTimeFromDateTime(const std::string& date_time_string);

std::chrono::duration<double, std::ratio<60>> GetChronoTimeInMinutesFromTime(const std::string& time_string);

std::string GetHourPartFromDateTime(const std::string& date_time);

std::string GetHourPartFromDateTimeAndSum(const std::string& start_time, const std::string& end_time);

std::vector<Node> ParseWhenMultiNode(const Json::Value& nodes);

Node ParseWhenSingleNode(const Json::Value& node);

}  // namespace parser

void WriteStringToFile(const std::string& string_to_write);

std::vector<std::string> GetCurrentInferenceResultInJsonFormat(const std::vector<InfeeredResult>& result,
                                                               const std::string& type,
                                                               const std::string& tree,
                                                               const std::string& id,
                                                               const bool is_test);

std::vector<Json::Value> ConvertCurrentInferenceResultToJson(const std::vector<InfeeredResult>& result,
                                                             const std::string& type,
                                                             const std::string& tree,
                                                             const std::string& id);

Json::Value GroupMessages(const Json::Value& message_1, const Json::Value& message2);


std::string GetIntials(const std::string input_string);


template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 15)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}
}  // namespace utils

#endif
