#include "metadata.h"

/**
 * @brief Constructs a Metadata object with optional timestamps for when the
 * data was received and generated.
 *
 * @param timestamp_received An optional timestamp indicating when the data was
 * received. If not provided, the current system time is used.
 * @param timestamp_generated An optional timestamp indicating when the data was
 * generated.
 * @param confidence An optional confidence value associated with the metadata.
 */
Metadata::Metadata(const Timestamps &timestamps, const std::optional<OriginType> &origin,
                   const std::optional<std::pair<ConfidenceType, std::string>> &confidence)
    : generated_(timestamps.generated), origin_(origin), confidence_(confidence) {
    if (timestamps.received.has_value()) {
        received_ = timestamps.received.value();
    } else {
        received_ = std::chrono::system_clock::now();
    }
}

/**
 * @brief Retrieves the generated time point of the metadata.
 *
 * @return An optional containing the time point when the metadata was
 * generated. If the time point is not set, the optional will be empty.
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

/**
 * @brief Retrieves the system origin information of the data.
 *
 * @return An optional containing the origin information. If the origin is not
 *         set, the optional will be empty.
 */
std::optional<Metadata::OriginType> Metadata::getOriginType() const { return origin_; }

/**
 * @brief Retrieves the confidence value associated with the metadata.
 *
 * @return An optional containing a pair of confidence type and value.
 *         If the confidence is not set, the optional will be empty.
 */
std::optional<std::pair<ConfidenceType, std::string>> Metadata::getConfidence() const {
    return confidence_;
}