#ifndef HELPER_H
#define HELPER_H

#include <chrono>
#include <ctime>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "coordinates_types.h"

class Helper {
   public:
    static std::string getFormattedTimestampNow(const std::string& format,
                                                bool include_nanoseconds = false,
                                                bool use_utc = true);
    static std::string getFormattedTimestampCustom(
        const std::string& format, const std::chrono::system_clock::time_point& timestamp,
        bool include_milliseconds = false, bool use_utc = true);
    static std::string extractNanoseconds(const std::chrono::system_clock::time_point& timestamp);

    static std::chrono::nanoseconds getNanosecondsSinceEpoch(
        const std::chrono::system_clock::time_point& timestamp);

    static std::pair<int64_t, int64_t> getSecondsAndNanosecondsSinceEpoch(
        const std::chrono::system_clock::time_point& timestamp);

    static std::tuple<std::optional<std::tm>, std::optional<int>> parseISO8601ToTime(
        const std::string& iso_string);

    static std::optional<NtmCoord> getCoordInNtm(const std::string& latitude,
                                                 const std::string& longitude);

    static std::string getEnvVariable(
        const std::string& env_var, const std::optional<std::string>& default_value = std::nullopt);

    static std::string toLowerCase(const std::string& input);
    static std::string toUppercase(const std::string& input);
    static std::string trimTrailingNewlines(const std::string& str);
    static nlohmann::json detectType(const std::string& value);
    static std::vector<std::string> splitString(const std::string& str, char delimiter);
    static std::string jsonToString(const nlohmann::json& json);
    static std::string variantToString(
        const std::variant<std::string, int, double, float, bool>& var);
    static std::chrono::system_clock::time_point convertToTimestamp(int64_t seconds, int64_t nanos);

   private:
    static const Wgs84Coord ZONE_ORIGIN;
    static std::string formatTimeT(bool use_utc, std::time_t& time_t, const std::string& format,
                                   std::optional<std::string> nanos);
};
#endif  // HELPER_H