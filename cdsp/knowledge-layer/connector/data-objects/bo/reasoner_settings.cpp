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
 * @param frames A boolean indicating whether the Triple Assembler groups the signals of one
 * message into one frame. Which inference engines support it is checked when the model
 * configuration is read.
 * @param frame_max_value_age The maximum age of a value in a frame, before the frame time. Older
 * values are left out of the frame. Must be positive.
 *
 * @throws std::invalid_argument if the supported schema collections vector is empty or the
 * maximum age of a frame value is not positive.
 */
ReasonerSettings::ReasonerSettings(const InferenceEngineType& inference_engine,
                                   const ReasonerSyntaxType& output_format,
                                   std::vector<SchemaType> supported_schema_collections,
                                   const bool is_ai_reasoner_inference_results, const bool frames,
                                   const std::chrono::milliseconds frame_max_value_age)
    : inference_engine_(inference_engine),
      output_format_(output_format),
      supported_schema_collections_(supported_schema_collections),
      is_ai_reasoner_inference_results_(is_ai_reasoner_inference_results),
      frames_(frames),
      frame_max_value_age_(frame_max_value_age) {
    if (supported_schema_collections_.empty()) {
        throw std::invalid_argument("Supported schema collections cannot be empty");
    }
    if (frame_max_value_age_.count() <= 0) {
        throw std::invalid_argument("The maximum age of a frame value must be positive");
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

/**
 * @brief Checks if frames are enabled.
 *
 * With frames, the Triple Assembler groups the signals of one message into one frame
 * (a sosa:ObservationCollection) instead of one observation per signal.
 *
 * @return true if frames are enabled, false otherwise.
 */
bool ReasonerSettings::isFramesEnabled() const { return frames_; }

/**
 * @brief Retrieves the maximum age of a value in a frame.
 *
 * Values older than this before the frame time are left out of the frame.
 *
 * @return The maximum age of a frame value.
 */
std::chrono::milliseconds ReasonerSettings::getFrameMaxValueAge() const {
    return frame_max_value_age_;
}
