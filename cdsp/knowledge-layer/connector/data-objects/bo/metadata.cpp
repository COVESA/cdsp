#include "metadata.h"

#include "helper.h"

/**
 * @brief Constructs a Metadata object with optional timestamps for when the data was received and
 * generated.
 *
 * @param timestamp_received An optional timestamp indicating when the data was received.
 *                           If not provided, the current system time is used.
 * @param timestamp_generated An optional timestamp indicating when the data was generated.
 */
Metadata::Metadata(const std::optional<std::chrono::system_clock::time_point>& timestamp_received,
                   const std::optional<std::chrono::system_clock::time_point>& timestamp_generated)
    : generated_(timestamp_generated) {
    if (timestamp_received.has_value()) {
        received_ = timestamp_received.value();
    } else {
        received_ = std::chrono::system_clock::now();
    }
}

/**
 * @brief Retrieves the generated time point of the metadata.
 *
 * @return An optional containing the time point when the metadata was generated.
 *         If the time point is not set, the optional will be empty.
 */
std::optional<std::chrono::system_clock::time_point> Metadata::getGenerated() const {
    return generated_;
}

/**
 * @brief Retrieves the received time point of the metadata.
 *
 * @return The time point when the metadata was received.
 */
std::chrono::system_clock::time_point Metadata::getReceived() const { return received_; }