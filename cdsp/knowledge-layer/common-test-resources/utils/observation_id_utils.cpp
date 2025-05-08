#include "observation_id_utils.h"

#include <iostream>

#include "helper.h"

/**
 * @brief Creates an observation identifier based on the timestamp and an identifier counter.
 *
 * This function generates an observation identifier based on the provided timestamp and an
 * identifier counter. The identifier is created by concatenating the timestamp in the format
 * "YYYYMMDDHHMMSSnnnnnnnnn" with the identifier counter.
 *
 * @param timestamp The timestamp to use for the observation identifier.
 * @return The observation identifier.
 */
const std::string ObservationIdentifier::createObservationIdentifier(
    const std::chrono::system_clock::time_point& timestamp) {
    std::string identifier = Helper::getFormattedTimestampCustom("%Y%m%d%H%M%S", timestamp, true);
    size_t dot_pos = identifier.find('.');
    if (dot_pos != std::string::npos) {
        identifier = identifier.substr(0, dot_pos) + identifier.substr(dot_pos + 1);
    }
    return identifier;
}
