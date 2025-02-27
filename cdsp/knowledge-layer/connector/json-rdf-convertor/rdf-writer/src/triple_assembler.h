#ifndef TRIPLE_ASSEMBLER_H
#define TRIPLE_ASSEMBLER_H

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "coordinates_types.h"
#include "data_message.h"
#include "data_types.h"
#include "i_file_handler.h"
#include "node.h"
#include "rdfox_adapter.h"
#include "triple_writer.h"

using chrono_time_nanoseconds = std::chrono::nanoseconds;

struct CoordinateNodes {
    Node latitude;
    Node longitude;
};

class TripleAssembler {
   public:
    TripleAssembler(const ModelConfig& model_config, RDFoxAdapter& adapter,
                    IFileHandler& file_reader, TripleWriter& triple_writer);

    void initialize();

    void transformMessageToRDFTriple(const DataMessage& message);

    ~TripleAssembler() = default;

   protected:
    void generateTriplesFromNode(const Node& node, const SchemaType& msg_schema_type,
                                 const std::optional<double>& ntm_coord_value = std::nullopt);

    void generateTriplesFromCoordinates(std::optional<CoordinateNodes>& valid_coordinates,
                                        const SchemaType& msg_schema_type,
                                        const DataMessage& message);

    void storeTripleOutput(const std::string& triple_output);

   private:
    RDFoxAdapter& rdfox_adapter_;
    IFileHandler& file_handler_;
    TripleWriter& triple_writer_;

    chrono_time_nanoseconds coordinates_last_time_stamp_{chrono_time_nanoseconds(0)};

    std::map<chrono_time_nanoseconds, std::unordered_map<std::string, Node>>
        timestamp_coordinates_messages_map_{};

    const ModelConfig& model_config_;

    const std::vector<std::map<std::string, std::string>> json_data_;

    std::pair<std::vector<std::string>, std::string> extractObjectsAndDataElements(
        const std::string& node_name);

    std::optional<CoordinateNodes> getValidCoordinatesPair();

    void cleanupOldTimestamps();

    std::pair<std::string, std::tuple<std::string, std::string, std::string>>
    getQueryPrefixesAndData(const SchemaType& msg_schema_type, const std::string& property_type,
                            const std::string& subject_class, const std::string& object_class);

    std::string getQueryFromFilePath(const SchemaType& query_key, const std::string& property_type);

    void replaceAllSparqlVariables(std::string& query, const std::string& from,
                                   const std::string& to);

    std::tuple<std::string, std::string, std::string> extractElementValuesFromQuery(
        const std::string& input);

    std::string extractPrefixesFromQuery(const std::string& query);

    std::string getFileExtension();

    const std::chrono::system_clock::time_point getTimestampFromNode(const Node& node);
};

#endif  // TRIPLE_ASSEMBLER_H