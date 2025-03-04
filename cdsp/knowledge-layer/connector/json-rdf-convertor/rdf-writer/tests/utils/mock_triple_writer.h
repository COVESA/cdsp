#ifndef MOCK_TRIPLE_WRITER_H
#define MOCK_TRIPLE_WRITER_H

#include <gmock/gmock.h>

#include "triple_writer.h"

class MockTripleWriter : public TripleWriter {
   public:
    MOCK_METHOD(void, initiateTriple, (const std::string&), (override));
    MOCK_METHOD(void, addElementObjectToTriple,
                (const std::string&, (const std::tuple<std::string, std::string, std::string>&) ),
                (override));

    MOCK_METHOD(void, addElementDataToTriple,
                ((const std::string&), (const std::tuple<std::string, std::string, std::string>&),
                 (const std::string&), (const std::chrono::system_clock::time_point&),
                 (const std::optional<double>&) ),
                (override));
    MOCK_METHOD(std::string, generateTripleOutput, (const ReasonerSyntaxType&), (override));
};

#endif  // MOCK_TRIPLE_WRITER_H