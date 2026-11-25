#include "frame_builder.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "helper.h"

namespace {
const std::string LATITUDE = "Vehicle.CurrentLocation.Latitude";
const std::string LONGITUDE = "Vehicle.CurrentLocation.Longitude";
}  // namespace

/**
 * @brief Constructs a FrameBuilder.
 *
 * @param triple_writer The writer used to build the frame triples.
 * @param max_value_age The maximum age of a value before the frame time, from the reasoner
 * settings. Older values are left out of the frame.
 *
 * @throws std::invalid_argument If the maximum age is not positive.
 */
FrameBuilder::FrameBuilder(TripleWriter& triple_writer, std::chrono::milliseconds max_value_age)
    : triple_writer_(triple_writer), max_value_age_(max_value_age) {
    if (max_value_age_.count() <= 0) {
        throw std::invalid_argument("The maximum age of a frame value must be positive");
    }
}

/**
 * @brief Adds the signals of one message and returns the frame of the previous moment, if the
 * message belongs to a later moment.
 *
 * The time of a message is the latest timestamp of its signals. Messages with the same time
 * belong to one moment and fill the same frame. The first message with a later time ends that
 * moment: its frame is built from the values before this message, then the values of the message
 * are kept for the next frame. The frame of the latest moment is therefore built only when newer
 * data arrives. A message older than the open moment only updates the kept values.
 *
 * @param identifier The vehicle identifier (VIN) from the message header.
 * @param signals The data point name and the resolved values of each signal of the message.
 * @param format The RDF syntax of the frame.
 * @return The frame of the previous moment, or nothing if the moment is still open or the frame
 * has no recent value.
 *
 * @throws std::invalid_argument If a value is empty.
 */
std::optional<BuiltFrame> FrameBuilder::addMessage(
    const std::string& identifier,
    const std::vector<std::pair<std::string, ResolvedSignal>>& signals,
    const ReasonerSyntaxType& format) {
    if (signals.empty()) {
        return std::nullopt;
    }
    auto message_time = signals.front().second.timestamp;
    for (const auto& [signal_name, signal] : signals) {
        if (signal.value.empty()) {
            throw std::invalid_argument("Frame signal value cannot be empty: " + signal_name);
        }
        message_time = std::max(message_time, signal.timestamp);
    }

    std::optional<BuiltFrame> frame;
    const auto open_time = open_frame_times_.find(identifier);
    if (open_time == open_frame_times_.end()) {
        open_frame_times_.emplace(identifier, message_time);
    } else if (message_time > open_time->second) {
        std::string triples = buildFrame(identifier, open_time->second, format);
        if (!triples.empty()) {
            frame = BuiltFrame{std::move(triples), open_time->second};
        }
        open_time->second = message_time;
    }

    for (const auto& [signal_name, signal] : signals) {
        updateSignal(identifier, signal_name, signal);
    }
    return frame;
}

/**
 * @brief Keeps the value of a signal if it is not older than the value already kept for the
 * same vehicle.
 *
 * @param identifier The vehicle identifier (VIN) from the message header.
 * @param signal_name The data point name of the signal, for example
 * `Vehicle.Chassis.SteeringWheel.Angle`.
 * @param signal The resolved values of the signal.
 *
 * @throws std::invalid_argument If the value is empty.
 */
void FrameBuilder::updateSignal(const std::string& identifier, const std::string& signal_name,
                                const ResolvedSignal& signal) {
    if (signal.value.empty()) {
        throw std::invalid_argument("Frame signal value cannot be empty: " + signal_name);
    }
    auto& signals = latest_signals_[identifier];
    const auto found = signals.find(signal_name);
    if (found == signals.end()) {
        signals.emplace(signal_name, signal);
    } else if (signal.timestamp >= found->second.timestamp) {
        found->second = signal;
    }
}

/**
 * @brief Builds one frame from the latest value of each signal of one vehicle.
 *
 * Values older than the maximum age before the frame time are left out. Latitude and longitude are
 * only taken together, with their NTM values.
 *
 * @param identifier The vehicle identifier (VIN): selects the kept values and is used in the
 * instance IRIs.
 * @param frame_time The time of the frame, the latest timestamp of the message.
 * @param format The RDF syntax of the output.
 * @return The frame as RDF, or an empty string if no signal is recent enough.
 */
std::string FrameBuilder::buildFrame(const std::string& identifier,
                                     const std::chrono::system_clock::time_point& frame_time,
                                     const ReasonerSyntaxType& format) {
    const auto vehicle = latest_signals_.find(identifier);
    if (vehicle == latest_signals_.end()) {
        return "";
    }
    const auto& signals = vehicle->second;

    const auto is_recent = [this, &frame_time](const ResolvedSignal& signal) {
        return frame_time - signal.timestamp <= max_value_age_;
    };
    const auto latitude = signals.find(LATITUDE);
    const auto longitude = signals.find(LONGITUDE);
    std::optional<NtmCoord> location_ntm;
    if (latitude != signals.end() && longitude != signals.end() && is_recent(latitude->second) &&
        is_recent(longitude->second)) {
        try {
            location_ntm = Helper::getCoordInNtm(latitude->second.value, longitude->second.value);
        } catch (const std::exception& e) {
            std::cerr << "The location is left out of the frame: " << e.what() << std::endl;
        }
    }

    // Each member with its NTM value, which only latitude and longitude have
    std::vector<std::pair<const ResolvedSignal*, std::optional<double>>> members;
    for (const auto& [signal_name, signal] : signals) {
        if (signal_name == LATITUDE || signal_name == LONGITUDE) {
            if (location_ntm.has_value()) {
                members.emplace_back(&signal, signal_name == LATITUDE ? location_ntm->northing
                                                                      : location_ntm->easting);
            }
        } else if (is_recent(signal)) {
            members.emplace_back(&signal, std::nullopt);
        }
    }
    if (members.empty()) {
        return "";
    }

    triple_writer_.initiateTriple(identifier);
    std::vector<std::string> member_uris;
    for (const auto& [signal, ntm_value] : members) {
        for (const auto& [prefixes, object_values] : signal->object_elements) {
            triple_writer_.addElementObjectToTriple(prefixes, object_values);
        }
        member_uris.push_back(triple_writer_.addMemberDataToTriple(
            signal->data_element.first, signal->data_element.second, signal->value,
            signal->timestamp, frame_time, ntm_value));
    }

    // The frame belongs to the root class of the signals, for example the vehicle
    const ResolvedSignal* first = members.front().first;
    const std::string root_class = first->object_elements.empty()
                                       ? std::get<0>(first->data_element.second)
                                       : std::get<0>(first->object_elements.front().second);
    triple_writer_.addFrameToTriple(root_class, frame_time, member_uris);

    return triple_writer_.generateTripleOutput(format);
}
