#ifndef METADATA_H
#define METADATA_H

#include <chrono>
#include <optional>

#include "data_types.h"

class Metadata {
   public:
    struct Timestamps {
        std::optional<std::chrono::system_clock::time_point> received;
        std::optional<std::chrono::system_clock::time_point> generated;
    };

    struct OriginType {
        std::optional<std::string> name;
        std::optional<std::string> uri;
    };
    explicit Metadata(
        const Timestamps &timestamps = {std::nullopt, std::nullopt},
        const std::optional<OriginType> &origin = std::nullopt,
        const std::optional<std::pair<ConfidenceType, std::string>> &confidence = std::nullopt);

    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> getGenerated() const;
    [[nodiscard]] std::chrono::system_clock::time_point getReceived() const;
    [[nodiscard]] std::optional<OriginType> getOriginType() const;
    [[nodiscard]] std::optional<std::pair<ConfidenceType, std::string>> getConfidence() const;

   private:
    std::optional<std::chrono::system_clock::time_point> generated_;
    std::chrono::system_clock::time_point received_;
    std::optional<OriginType> origin_;
    std::optional<std::pair<ConfidenceType, std::string>> confidence_;
};

#endif  // METADATA_H