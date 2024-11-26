#ifndef TRIPLE_ASSEMBLER_H
#define TRIPLE_ASSEMBLER_H

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "../../utils/i_file_handler.h"
#include "data_types.h"
#include "rdfox_adapter.h"
#include "triple_writer.h"

class TripleAssembler {
   public:
    TripleAssembler(const ModelConfig& model_config, RDFoxAdapter& adapter,
                    IFileHandler& file_reader, TripleWriter& triple_writer);

    void initialize();

    virtual void transformMessageToRDFTriple(const DataMessage& message);

    ~TripleAssembler() = default;

   protected:
    virtual void generateTriplesFromNode(const Node& node, const std::string& msg_tree,
                                         const std::string& msg_date_time);

    virtual void storeTripleOutput(const std::string& triple_output);

   private:
    RDFoxAdapter& rdfox_adapter_;
    IFileHandler& file_handler_;
    TripleWriter& triple_writer_;
    const ModelConfig& model_config_;

    const std::vector<std::map<std::string, std::string>> json_data_;

    std::pair<std::vector<std::string>, std::string> extractObjectsAndDataElements(
        const std::string& node_name);

    std::pair<std::string, std::tuple<std::string, std::string, std::string>>

    getQueryPrefixesAndData(const std::string& message_tree, const std::string& property_type,
                            const std::string& subject_class, const std::string& object_class);

    std::string getQueryFromFilePath(const std::string& query_key,
                                     const std::string& property_type);

    void replaceAllSparqlVariables(std::string& query, const std::string& from,
                                   const std::string& to);

    std::tuple<std::string, std::string, std::string> extractElementValuesFromQuery(
        const std::string& input);

    std::string extractPrefixesFromQuery(const std::string& query);

    std::string getFileExtension();
};

#endif  // TRIPLE_ASSEMBLER_H