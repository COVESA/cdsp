#ifndef REASONER_SETTINGS_H
#define REASONER_SETTINGS_H

#include <string>
#include <vector>

#include "data_types.h"

class ReasonerSettings {
   public:
    ReasonerSettings(const InferenceEngineType& inference_engine,
                     const ReasonerSyntaxType& output_format,
                     std::vector<SchemaType> supported_schema_collections,
                     const bool is_ai_reasoner_inference_results);
    InferenceEngineType getInferenceEngine() const;
    ReasonerSyntaxType getOutputFormat() const;
    std::vector<SchemaType> getSupportedSchemaCollections() const;
    bool isIsAiReasonerInferenceResults() const;

   private:
    InferenceEngineType inference_engine_;
    ReasonerSyntaxType output_format_;
    std::vector<SchemaType> supported_schema_collections_;
    bool is_ai_reasoner_inference_results_;
};

#endif  // REASONER_SETTINGS_H