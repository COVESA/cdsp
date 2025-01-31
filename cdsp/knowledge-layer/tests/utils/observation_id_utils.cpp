#include "observation_id_utils.h"

#include "helper.h"

/**
 * @brief Function to create a unique observation identifier based on date-time and a counter
 */
const std::string ObservationIdentifier::createObservationIdentifier(const std::string& date_time,
                                                                     const int identifier_counter) {
    const auto [parsed_date_time, _] = Helper::parseISO8601ToTime(date_time);
    std::string identifier =
        Helper::getFormattedTimestamp("%Y%m%d%H%M%S", false, false, parsed_date_time);
    return identifier + "O" + std::to_string(identifier_counter);
}
