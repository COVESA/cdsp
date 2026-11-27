#ifndef FRAME_BUILDER_H
#define FRAME_BUILDER_H

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "data_types.h"
#include "resolved_signal.h"
#include "triple_writer.h"

/**
 * @brief A built frame and its time, also the event time of the stream element.
 */
struct BuiltFrame {
    std::string triples;
    std::chrono::system_clock::time_point time;
};

/**
 * @brief Builds frames: one stream element with the latest value of each input signal.
 *
 * Values are kept per vehicle identifier, so a frame only contains the values of its own vehicle.
 * The frame is a `sosa:ObservationCollection` with one member observation (`sosa:hasMember`) per
 * signal, and it also carries the value of each member, so a query can read the values on the
 * frame without joining the members. There is one frame per moment: the signals of one moment can arrive in several messages,
 * so the frame of a moment is built when the first message of a later moment arrives. Used when
 * the reasoner settings contain "frames"; which inference engines support frames is checked when
 * the model configuration is read.
 */
class FrameBuilder {
   public:
    FrameBuilder(TripleWriter& triple_writer, std::chrono::milliseconds max_value_age);
    virtual ~FrameBuilder() = default;

    virtual std::optional<BuiltFrame> addMessage(
        const std::string& identifier,
        const std::vector<std::pair<std::string, ResolvedSignal>>& signals,
        const ReasonerSyntaxType& format);
    virtual void updateSignal(const std::string& identifier, const std::string& signal_name,
                              const ResolvedSignal& signal);
    virtual std::string buildFrame(const std::string& identifier,
                                   const std::chrono::system_clock::time_point& frame_time,
                                   const ReasonerSyntaxType& format);

   private:
    TripleWriter& triple_writer_;
    // A value older than this before the frame time is left out of the frame
    std::chrono::milliseconds max_value_age_;
    // Latest value of each signal, per vehicle identifier
    std::map<std::string, std::map<std::string, ResolvedSignal>> latest_signals_;
    // Time of the moment whose frame is not built yet, per vehicle identifier
    std::map<std::string, std::chrono::system_clock::time_point> open_frame_times_;
};

#endif  // FRAME_BUILDER_H
