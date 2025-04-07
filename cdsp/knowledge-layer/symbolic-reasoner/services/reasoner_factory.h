#ifndef REASONER_FACTORY_H
#define REASONER_FACTORY_H

#include <memory>

#include "data_types.h"
#include "i_reasoner_adapter.h"
#include "rdfox_adapter.h"
#include "reasoner_service.h"

class ReasonerFactory {
   public:
    static std::shared_ptr<ReasonerService> initReasoner(
        const InferenceEngineType& inference_engine, const ReasonerServerData& server_data,
        const std::vector<std::pair<RuleLanguageType, std::string>>& reasoner_rules,
        const std::vector<std::pair<ReasonerSyntaxType, std::string>>& ontologies,
        const bool reset_datastore);

   protected:
    static void loadRules(const std::shared_ptr<ReasonerService>& reasoner_service,
                          const std::vector<std::pair<RuleLanguageType, std::string>>& rules);
    static void loadOntologies(
        const std::shared_ptr<ReasonerService>& reasoner_service,
        const std::vector<std::pair<ReasonerSyntaxType, std::string>>& ontologies);
};

#endif  // REASONER_FACTORY_H