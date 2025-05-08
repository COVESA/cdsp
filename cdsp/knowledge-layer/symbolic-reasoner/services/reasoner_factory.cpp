#include "reasoner_factory.h"

#include <iostream>

/**
 * Initializes a ReasonerService based on the specified inference engine, server data,
 * reasoner rules, and ontologies.
 *
 * @param inference_engine The type of inference engine to be used.
 * @param server_data The server data required for initializing the reasoner.
 * @param reasoner_rules A vector of pairs containing rule language types and their corresponding
 * rules.
 * @param ontologies A vector of pairs containing reasoner syntax types and their corresponding
 * ontologies.
 * @return A shared pointer to the initialized ReasonerService.
 * @throws std::invalid_argument If the specified inference engine is unsupported.
 * @throws std::runtime_error If the reasoner service fails to initialize due to a missing data
 * store.
 */
std::shared_ptr<ReasonerService> ReasonerFactory::initReasoner(
    const InferenceEngineType& inference_engine, const ReasonerServerData& server_data,
    const std::vector<std::pair<RuleLanguageType, std::string>>& reasoner_rules,
    const std::vector<std::pair<ReasonerSyntaxType, std::string>>& ontologies,
    const bool reset_datastore) {
    std::shared_ptr<IReasonerAdapter> reasoner_adapter;

    if (inference_engine == InferenceEngineType::RDFOX) {
        reasoner_adapter = std::make_shared<RDFoxAdapter>(server_data);
    } else {
        throw std::invalid_argument("Unsupported inference engine in `model_config.json`.");
    }

    std::shared_ptr<ReasonerService> reasoner_service;

    reasoner_service = std::make_shared<ReasonerService>(reasoner_adapter, reset_datastore);

    if (!reasoner_service->checkDataStore()) {
        throw std::runtime_error(
            "Failed to initialize the reasoner service. Data store not found.");
    }

    loadRules(reasoner_service, reasoner_rules);
    loadOntologies(reasoner_service, ontologies);
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

/**
 * @brief Loads a set of ontologies into the ReasonerService.
 *
 * This function attempts to load a collection of ontologies into the provided
 * ReasonerService. Each ontology is represented as a pair consisting of a
 * ReasonerSyntaxType and a string. The function counts and outputs the number
 * of successfully loaded ontologies.
 *
 * @param reasoner_service A shared pointer to the ReasonerService where the
 *                        ontologies will be loaded. Must be initialized.
 * @param ontologies A vector of pairs, where each pair contains a ReasonerSyntaxType
 *                   and a string representing the ontology to be loaded.
 */
void ReasonerFactory::loadOntologies(
    const std::shared_ptr<ReasonerService>& reasoner_service,
    const std::vector<std::pair<ReasonerSyntaxType, std::string>>& ontologies) {
    std::cout << " - Loading ontologies into the reasoner service..." << std::endl;
    for (const auto& ontology : ontologies) {
        if (!reasoner_service->loadData(ontology.second, ontology.first)) {
            throw std::runtime_error("Failed to load ontologies into the reasoner service.");
        }
    }
    std::cout << " - Successfully loaded ontologies!" << std::endl;
}