#ifndef METADATA_H
#define METADATA_H

#include <chrono>
#include <optional>

class Metadata {
   public:
    explicit Metadata(
        const std::optional<std::chrono::system_clock::time_point>& timestamp_received =
            std::nullopt,
        const std::optional<std::chrono::system_clock::time_point>& timestamp_generated =
            std::nullopt);

    std::optional<std::chrono::system_clock::time_point> getGenerated() const;
    std::chrono::system_clock::time_point getReceived() const;

   private:
    std::optional<std::chrono::system_clock::time_point> generated_;
    std::chrono::system_clock::time_point received_;
};

#endif  // METADATA_H