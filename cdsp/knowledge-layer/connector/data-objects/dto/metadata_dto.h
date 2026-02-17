#ifndef METADATA_DTO_H
#define METADATA_DTO_H

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>

/**
 * @brief Data Transfer Object for Metadata
 */
struct MetadataDTO {
    struct Timestamp {
        int64_t seconds = 0;
        int64_t nanos = 0;
    };

    struct OriginType {
        std::string name;
        std::optional<std::string> uri;
    };

    struct Confidence {
        std::string type;
        int value = 0;
    };

    struct NodeMetadata {
        Timestamp received;
        Timestamp generated;
        std::optional<OriginType> origin_type;
        std::optional<Confidence> confidence;
    };

    std::unordered_map<std::string, NodeMetadata> nodes;

    // Overload the << operator to print the DTO

    friend std::ostream &operator<<(std::ostream &out_stream, const MetadataDTO &dto) {
        out_stream << "MetadataDTO: {\n";
        for (const auto &node : dto.nodes) {
            out_stream << "  " << node.first << ": {\n";
            out_stream << "    timestamps: {";
            out_stream << "      received: { Seconds: " << node.second.received.seconds << ", "
                       << " nanos: " << node.second.received.nanos << " },\n";
            out_stream << "      generated: { Seconds: " << node.second.generated.seconds << ", "
                       << " nanos: " << node.second.generated.nanos << " }\n";
            out_stream << "    }\n";

            if (node.second.origin_type.has_value() && node.second.origin_type->name != "") {
                out_stream << "    origin: { "
                           << "       type: { name: " << node.second.origin_type->name;
                if (node.second.origin_type->uri) {
                    out_stream << ", uri: " << *node.second.origin_type->uri;
                } else {
                    out_stream << ", uri: <empty>";
                }
                out_stream << "       } \n"
                           << "    }\n";
            }

            if (node.second.confidence) {
                out_stream << "    confidence: { type: " << node.second.confidence->type
                           << ", value: " << node.second.confidence->value << " }\n";
            }
            out_stream << "  }\n";
        }
        out_stream << "}";
        return out_stream;
    }
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a MetadataDTO object into a JSON representation.
 *
 * @param json_object The JSON object to which the MetadataDTO will be serialized.
 * @param dto The MetadataDTO object containing the data to be serialized.
 */
inline void to_json(nlohmann::json &json_object, const MetadataDTO &dto) {
    json_object = nlohmann::json::object();
    for (const auto &[node_name, metadata] : dto.nodes) {
        if (metadata.received.seconds != 0 || metadata.received.nanos != 0) {
            json_object[node_name]["timestamps"]["received"] = {
                {"seconds", metadata.received.seconds}, {"nanos", metadata.received.nanos}};
        }
        if (metadata.generated.seconds != 0 || metadata.generated.nanos != 0) {
            json_object[node_name]["timestamps"]["generated"] = {
                {"seconds", metadata.generated.seconds}, {"nanos", metadata.generated.nanos}};
        }
        if (metadata.origin_type.has_value() && metadata.origin_type->name != "") {
            json_object[node_name]["origin"]["type"] = {{"name", metadata.origin_type->name}};
            if (metadata.origin_type->uri) {
                json_object[node_name]["origin"]["type"]["uri"] = *metadata.origin_type->uri;
            } else {
                json_object[node_name]["origin"]["type"]["uri"] = "";
            }
        }
        if (metadata.confidence) {
            json_object[node_name]["confidence"] = {{"type", metadata.confidence->type},
                                                    {"value", metadata.confidence->value}};
        }
    }
}

#endif  // METADATA_DTO_H