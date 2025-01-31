#ifndef HELPER_H
#define HELPER_H

#include <chrono>
#include <ctime>
#include <optional>
#include <string>
#include <tuple>

#include "coordinates_types.h"
#include "nlohmann/json.hpp"

namespace Helper {
// TODO: Should be a more generic geographical point?
constexpr Wgs84Coord ZONE_ORIGIN{11.579144, 48.137416, 0.0};
std::string getFormattedTimestamp(const std::string& format, bool include_milliseconds = false,
                                  bool use_utc = true,
                                  const std::optional<std::tm>& tm = std::nullopt,
                                  const std::optional<int>& milliseconds = std::nullopt);
std::optional<NtmCoord> getCoordInNtm(const std::string& latitude, const std::string& longitude);
std::tuple<std::optional<std::tm>, std::optional<int>> parseISO8601ToTime(
    const std::string& iso_string);
std::chrono::duration<double, std::milli> getMillisecondsSinceEpoch(const std::string& iso_string);
std::string toLowerCase(const std::string& input);
std::string toUppercase(const std::string& input);
std::string trimTrailingNewlines(const std::string& str);
nlohmann::json detectType(const std::string& value);
}  // namespace Helper
#endif  // HELPER_H