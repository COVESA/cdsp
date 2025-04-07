#include "reasoner_settings.h"

/**
 * @brief Constructs a ReasonerSettings object with specified inference engine, output format, and
 * supported schema collections.
 *
 * @param inference_engine The type of inference engine to be used.
 * @param output_format The RDF syntax type for the output format.
 * @param supported_schema_collections A vector of schema types that are supported. Must not be
 * empty.
 * @param is_ai_reasoner_inference_results A boolean indicating whether the reasoning query results
 * should be grouped as inference.
 *
 * @throws std::invalid_argument if the supported schema collections vector is empty.
 */
ReasonerSettings::ReasonerSettings(const InferenceEngineType& inference_engine,
                                   const ReasonerSyntaxType& output_format,
                                   std::vector<SchemaType> supported_schema_collections,
                                   const bool is_ai_reasoner_inference_results)
    : inference_engine_(inference_engine),
      output_format_(output_format),
      supported_schema_collections_(supported_schema_collections),
      is_ai_reasoner_inference_results_(is_ai_reasoner_inference_results) {
    if (supported_schema_collections_.empty()) {
        throw std::invalid_argument("Supported schema collections cannot be empty");
    }
}

/**
 * @brief Retrieves the current inference engine type.
 *
 * This function returns the type of inference engine that is currently set
 * in the ReasonerSettings.
 *
 * @return InferenceEngineType The type of the inference engine.
 */
InferenceEngineType ReasonerSettings::getInferenceEngine() const { return inference_engine_; }

/**
 * @brief Retrieves the current output format for the reasoner settings.
 *
 * This function returns the reasoner syntax type that is currently set as the
 * output format in the reasoner settings.
 *
 * @return ReasonerSyntaxType The current output format.
 */
ReasonerSyntaxType ReasonerSettings::getOutputFormat() const { return output_format_; }

/**
 * @brief Retrieves the supported schema collections.
 *
 * This function returns a vector containing the schema types that are
 * supported by the ReasonerSettings.
 *
 * @return A vector of SchemaType representing the supported schema collections.
 */
std::vector<SchemaType> ReasonerSettings::getSupportedSchemaCollections() const {
    return supported_schema_collections_;
}

/**
 * @brief Checks if the inference of the reasoning query results.
 *
 * This function returns a boolean indicating whether the reasoning query results
 * should be grouped as inference.
 *
 * @return true if the reasoning query results should be inference, false otherwise.
 */
bool ReasonerSettings::isIsAiReasonerInferenceResults() const {
    return is_ai_reasoner_inference_results_;
}