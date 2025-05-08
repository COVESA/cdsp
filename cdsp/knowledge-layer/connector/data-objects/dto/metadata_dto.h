#ifndef METADATA_DTO_H
#define METADATA_DTO_H

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

    struct NodeMetadata {
        Timestamp received;
        Timestamp generated;
    };

    std::unordered_map<std::string, NodeMetadata> nodes;

    // Overload the << operator to print the DTO

    friend std::ostream& operator<<(std::ostream& os, const MetadataDTO& dto) {
        os << "MetadataDTO: {\n";
        for (const auto& node : dto.nodes) {
            os << "  " << node.first << ": {\n";
            os << "    received: { Seconds: " << node.second.received.seconds << ", "
               << " nanos: " << node.second.received.nanos << " },\n";
            os << "    generated: { Seconds: " << node.second.generated.seconds << ", "
               << " nanos: " << node.second.generated.nanos << " }\n";
            os << "  }\n";
        }
        os << "}";
        return os;
    }
};

/**
 * @brief Define the `to_json` function for `nlohmann::json`
 *
 * This function converts a MetadataDTO object into a JSON representation.
 *
 * @param j The JSON object to which the MetadataDTO will be serialized.
 * @param dto The MetadataDTO object containing the data to be serialized.
 */
inline void to_json(nlohmann::json& j, const MetadataDTO& dto) {
    j = nlohmann::json::object();
    for (const auto& [node_name, metadata] : dto.nodes) {
        if (metadata.received.seconds != 0 || metadata.received.nanos != 0) {
            j[node_name]["received"] = {{"seconds", metadata.received.seconds},
                                        {"nanos", metadata.received.nanos}};
        }
        if (metadata.generated.seconds != 0 || metadata.generated.nanos != 0) {
            j[node_name]["generated"] = {{"seconds", metadata.generated.seconds},
                                         {"nanos", metadata.generated.nanos}};
        }
    }
}

#endif  // METADATA_DTO_H