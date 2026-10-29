#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>

#include "mock_reasoner_adapter.h"
#include "reasoner_service.h"

using ::testing::Return;
using ::testing::StrEq;

/**
 * Reasoners that do not override loadObservations (RDFox, RDF4J) must receive the observations
 * through loadData, with the content type converted the same way as for loadData.
 */
TEST(ReasonerServiceUnitTest, LoadObservationsUsesLoadDataByDefault) {
    auto adapter = std::make_shared<MockReasonerAdapter>();
    EXPECT_CALL(*adapter, initialize());
    ReasonerService service(adapter, false);

    EXPECT_CALL(*adapter, loadData(StrEq("<s> <p> <o> ."), StrEq("text/turtle")))
        .WillOnce(Return(true));

    EXPECT_TRUE(service.loadObservations("<s> <p> <o> .", ReasonerSyntaxType::TURTLE,
                                         std::chrono::system_clock::now()));
}
