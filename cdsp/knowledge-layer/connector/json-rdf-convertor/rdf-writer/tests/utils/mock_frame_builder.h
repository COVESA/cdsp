#ifndef MOCK_FRAME_BUILDER_H
#define MOCK_FRAME_BUILDER_H

#include <gmock/gmock.h>

#include "frame_builder.h"

class MockFrameBuilder : public FrameBuilder {
   public:
    explicit MockFrameBuilder(TripleWriter& triple_writer)
        : FrameBuilder(triple_writer, std::chrono::milliseconds(1000)) {}

    MOCK_METHOD(std::optional<BuiltFrame>, addMessage,
                (const std::string&, (const std::vector<std::pair<std::string, ResolvedSignal>>&),
                 const ReasonerSyntaxType&),
                (override));
    MOCK_METHOD(void, updateSignal, (const std::string&, const std::string&, const ResolvedSignal&),
                (override));
    MOCK_METHOD(std::string, buildFrame,
                (const std::string&, const std::chrono::system_clock::time_point&,
                 const ReasonerSyntaxType&),
                (override));
};

#endif  // MOCK_FRAME_BUILDER_H
