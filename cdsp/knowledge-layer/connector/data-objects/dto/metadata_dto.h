#ifndef METADATA_DTO_H
#define METADATA_DTO_H

#include <string>
#include <unordered_map>

/**
 * @brief Data Transfer Object for Metadata
 */
struct MetadataDto {
    struct Timestamp {
        int64_t seconds = 0;
        int64_t nanoseconds = 0;
    };

    struct NodeMetadata {
        Timestamp received;
        Timestamp generated;
    };

    std::unordered_map<std::string, NodeMetadata> nodes;

    // Overload the << operator to print the DTO

    friend std::ostream& operator<<(std::ostream& os, const MetadataDto& dto) {
        os << "MetadataDto: {\n";
        for (const auto& node : dto.nodes) {
            os << "  " << node.first << ": {\n";
            os << "    received: { Seconds: " << node.second.received.seconds << ", "
               << " Nanoseconds: " << node.second.received.nanoseconds << " },\n";
            os << "    generated: { Seconds: " << node.second.generated.seconds << ", "
               << " Nanoseconds: " << node.second.generated.nanoseconds << " }\n";
            os << "  }\n";
        }
        os << "}";
        return os;
    }
};

#endif  // METADATA_DTO_H