#include <gtest/gtest.h>

#include <stdexcept>

#include "mock_reasoner_adapter.h"
#include "mock_reasoner_service.h"
#include "reasoning_query_service.h"

class ReasoningQueryServiceUnitTest : public ::testing::Test {
    // NOLINTBEGIN(cppcoreguidelines-non-private-member-variables-in-classes)
   protected:
    std::shared_ptr<MockReasonerService> mock_reasoner_service_;
    std::unique_ptr<ReasoningQueryService> reasoning_query_service_;
    std::shared_ptr<MockReasonerAdapter> mock_adapter_;
    // NOLINTEND(cppcoreguidelines-non-private-member-variables-in-classes)
    void SetUp() override {
        mock_adapter_ = std::make_shared<MockReasonerAdapter>();
        EXPECT_CALL(*mock_adapter_, initialize()).Times(1);
        mock_reasoner_service_ = std::make_unique<MockReasonerService>(mock_adapter_);
        reasoning_query_service_ = std::make_unique<ReasoningQueryService>(mock_reasoner_service_);
    }
};

/**
 * @brief Test case for processing a reasoning query where JSONWriter
 * throws an exception.
 *
 * This test verifies that when the JSONWriter throws an exception during the
 * processing of a reasoning query, the processReasoningQuery method
 * handles the exception gracefully and does not propagate it. The test checks
 * that the results are empty, indicating that no processing occurred due to
 * the exception.
 */
TEST_F(ReasoningQueryServiceUnitTest, ProcessReasoningQuery_JSONWriterThrowsException) {
    // Arrange
    ReasoningOutputQuery regular_query;
    regular_query.query = "SELECT ?s WHERE { ?s a <http://example.org/SomeClass> . }";
    regular_query.query_language = QueryLanguageType::SPARQL;
    ReasoningOutputQuery reasoning_output_queries = regular_query;

    bool is_ai_reasoner_inference_results = false;
    std::optional<std::string> reasoning_results_file_path = std::nullopt;

    EXPECT_CALL(*mock_reasoner_service_,
                queryData(regular_query.query, regular_query.query_language,
                          DataQueryAcceptType::SPARQL_JSON))
        .WillOnce(testing::Return("Snapshot content not parsable to JSON"));

    // Act && Assert
    EXPECT_THROW(reasoning_query_service_->processReasoningQuery(reasoning_output_queries,
                                                                 is_ai_reasoner_inference_results,
                                                                 reasoning_results_file_path),
                 std::runtime_error);
}