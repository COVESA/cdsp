#ifndef CONVERTER_HELPER_H
#define CONVERTER_HELPER_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

class ConverterHelper {
   public:
    static std::optional<std::chrono::system_clock::time_point> parseTimestamp(int64_t seconds,
                                                                               int64_t nanos);
};

#endif