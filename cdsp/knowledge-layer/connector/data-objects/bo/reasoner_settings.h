#ifndef REASONER_SETTINGS_H
#define REASONER_SETTINGS_H

#include <string>
#include <vector>

#include "data_types.h"

class ReasonerSettings {
   public:
    ReasonerSettings(const InferenceEngineType& inference_engine,
                     const ReasonerSyntaxType& output_format,
                     std::vector<SchemaType> supported_schema_collections);
    InferenceEngineType getInferenceEngine() const;
    ReasonerSyntaxType getOutputFormat() const;
    std::vector<SchemaType> getSupportedSchemaCollections() const;

   private:
    InferenceEngineType inference_engine_;
    ReasonerSyntaxType output_format_;
    std::vector<SchemaType> supported_schema_collections_;
};

#endif  // REASONER_SETTINGS_H