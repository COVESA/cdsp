#ifndef HELPER_H
#define HELPER_H

#include <ctime>
#include <string>
#include <tuple>

namespace Helper {
std::string getFormattedTimestamp(const std::string& format, bool includeMilliseconds = false,
                                  bool useUTC = true,
                                  const std::optional<std::tm>& tm = std::nullopt,
                                  const std::optional<int>& milliseconds = std::nullopt);
std::tuple<std::optional<std::tm>, std::optional<int>> parseISO8601ToTime(
    const std::string& isoString);
std::string toLowerCase(const std::string& input);
std::string toUppercase(const std::string& input);
std::string trimTrailingNewlines(const std::string& str);
}  // namespace Helper
#endif  // HELPER_H