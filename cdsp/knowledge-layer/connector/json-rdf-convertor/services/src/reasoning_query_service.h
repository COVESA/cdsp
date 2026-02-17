#ifndef REASONING_QUERY_SERVICE_H
#define REASONING_QUERY_SERVICE_H

#include <memory>
#include <optional>
#include <string>

#include "data_types.h"
#include "reasoner_service.h"

class ReasoningQueryService {
   public:
    ReasoningQueryService(std::shared_ptr<ReasonerService> reasoning_service);

    nlohmann::json processReasoningQuery(
        const ReasoningOutputQuery& reasoning_output_query,
        const bool is_ai_reasoner_inference_results = false,
        const std::optional<std::string>& output_file_path = std::nullopt);

   private:
    std::shared_ptr<ReasonerService> reasoning_service_;
    const std::optional<std::string> output_file_path_;
};

#endif  // REASONING_QUERY_SERVICE_H
