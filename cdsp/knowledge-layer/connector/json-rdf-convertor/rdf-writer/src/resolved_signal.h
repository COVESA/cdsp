#ifndef RESOLVED_SIGNAL_H
#define RESOLVED_SIGNAL_H

#include <chrono>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using RdfElementValues = std::tuple<std::string, std::string, std::string>;

/**
 * @brief One input signal with the values the Triple Assembler resolved through its helper
 * queries.
 */
struct ResolvedSignal {
    // Prefixes and object property values (class, object property, class) from the vehicle down to
    // the class of the signal
    std::vector<std::pair<std::string, RdfElementValues>> object_elements;
    // Prefixes and data property values (class, data property, datatype) of the signal
    std::pair<std::string, RdfElementValues> data_element;
    std::string value;
    std::chrono::system_clock::time_point timestamp;
};

#endif  // RESOLVED_SIGNAL_H
