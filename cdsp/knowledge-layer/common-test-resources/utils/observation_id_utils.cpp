#include "observation_id_utils.h"

#include "helper.h"

/**
 * @brief Creates an observation identifier based on the timestamp and an identifier counter.
 *
 * This function generates an observation identifier based on the provided timestamp and an
 * identifier counter. The identifier is created by concatenating the timestamp in the format
 * "YYYYMMDDHHMMSS" with the letter 'O' and the identifier counter.
 *
 * @param timestamp The timestamp to use for the observation identifier.
 * @param identifier_counter The identifier counter to use for the observation identifier.
 * @return The generated observation identifier as a string.
 */
const std::string ObservationIdentifier::createObservationIdentifier(
    const std::chrono::system_clock::time_point& timestamp, const int identifier_counter) {
    std::string identifier = Helper::getFormattedTimestampCustom("%Y%m%d%H%M%S", timestamp);
    return identifier + "O" + std::to_string(identifier_counter);
}
