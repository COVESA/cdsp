#include "reasoner_factory.h"

#include <iostream>

/**
 * @brief Initializes a ReasonerService with the specified inference engine.
 *
 * This function initializes a ReasonerService with the specified inference
 * engine and server data. It also loads a set of rules into the ReasonerService.
 * The function returns a shared pointer to the initialized ReasonerService.
 *
 * @param inference_engine The inference engine to be used by the ReasonerService.
 * @param server_data The server data required to connect to the inference engine.
 * @param reasoner_rules A vector of pairs, where each pair contains a RuleLanguageType
 *                       and a string representing the rule to be loaded.
 * @return A shared pointer to the initialized ReasonerService.
 */
std::shared_ptr<ReasonerService> ReasonerFactory::initReasoner(
    const InferenceEngineType& inference_engine, const ServerData& server_data,
    const std::vector<std::pair<RuleLanguageType, std::string>>& reasoner_rules) {
    std::shared_ptr<IReasonerAdapter> reasoner_adapter;

    if (inference_engine == InferenceEngineType::RDFOX) {
        reasoner_adapter = std::make_shared<RDFoxAdapter>(server_data);
    } else {
        throw std::invalid_argument("Unsupported inference engine in `model_config.json`.");
    }

    std::shared_ptr<ReasonerService> reasoner_service;

    reasoner_service = std::make_shared<ReasonerService>(reasoner_adapter);

    if (!reasoner_service->checkDataStore()) {
        throw std::runtime_error(
            "Failed to initialize the reasoner service. Data store not found.");
    }

    loadRules(reasoner_service, reasoner_rules);
    return reasoner_service;
}

/**
 * @brief Loads a set of rules into the ReasonerService.
 *
 * This function attempts to load a collection of rules into the provided
 * ReasonerService. Each rule is represented as a pair consisting of a
 * RuleLanguageType and a string. The function counts and outputs the number
 * of successfully loaded rules.
 *
 * @param reasoner_service A shared pointer to the ReasonerService where the
 *                         rules will be loaded. Must be initialized.
 * @param rules A vector of pairs, where each pair contains a RuleLanguageType
 *              and a string representing the rule to be loaded.
 */
void ReasonerFactory::loadRules(
    const std::shared_ptr<ReasonerService>& reasoner_service,
    const std::vector<std::pair<RuleLanguageType, std::string>>& rules) {
    std::cout << " - Loading rules into the reasoner service..." << std::endl;
    for (const auto& rule : rules) {
        if (!reasoner_service->loadRules(rule.second, rule.first)) {
            throw std::runtime_error("Failed to load rules into the reasoner service.");
        }
    }
    std::cout << " - Successfully loaded rules!" << std::endl;
}