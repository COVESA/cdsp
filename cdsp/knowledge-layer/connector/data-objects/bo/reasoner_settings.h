#ifndef REASONER_SETTINGS_H
#define REASONER_SETTINGS_H

#include <chrono>
#include <string>
#include <vector>

#include "data_types.h"

class ReasonerSettings {
   public:
    // Default maximum age of a value in a frame, before the frame time
    static constexpr std::chrono::milliseconds DEFAULT_FRAME_MAX_VALUE_AGE{1000};

    ReasonerSettings(
        const InferenceEngineType& inference_engine, const ReasonerSyntaxType& output_format,
        std::vector<SchemaType> supported_schema_collections,
        const bool is_ai_reasoner_inference_results, const bool frames = false,
        const std::chrono::milliseconds frame_max_value_age = DEFAULT_FRAME_MAX_VALUE_AGE);
    InferenceEngineType getInferenceEngine() const;
    ReasonerSyntaxType getOutputFormat() const;
    std::vector<SchemaType> getSupportedSchemaCollections() const;
    bool isIsAiReasonerInferenceResults() const;
    bool isFramesEnabled() const;
    std::chrono::milliseconds getFrameMaxValueAge() const;

   private:
    InferenceEngineType inference_engine_;
    ReasonerSyntaxType output_format_;
    std::vector<SchemaType> supported_schema_collections_;
    bool is_ai_reasoner_inference_results_;
    bool frames_;
    std::chrono::milliseconds frame_max_value_age_;
};

#endif  // REASONER_SETTINGS_H