#ifndef TRIPLE_WRITER_H
#define TRIPLE_WRITER_H

#include <serd/serd.h>

#include <chrono>
#include <optional>
#include <regex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "data_types.h"

struct TripleNodes {
    std::pair<SerdType, std::string> subject;
    std::pair<SerdType, std::string> predicate;
    std::pair<SerdType, std::string> object;
    std::optional<std::pair<SerdType, std::string>> datatype;
};

class TripleWriter {
   public:
    virtual void initiateTriple(const std::string& identifier);
    virtual void addElementObjectToTriple(
        const std::string& prefixes,
        const std::tuple<std::string, std::string, std::string>& rdf_object_values);
    virtual void addElementDataToTriple(
        const std::string& prefixes,
        const std::tuple<std::string, std::string, std::string>& rdf_data_values,
        const std::string& value, const std::chrono::system_clock::time_point& dataTime,
        const std::optional<double>& ntmValue = std::nullopt);

    virtual std::string generateTripleOutput(const ReasonerSyntaxType& format);

    ~TripleWriter() = default;

   private:
    std::string identifier_;

    std::unordered_map<std::string, std::string> unique_supported_prefixes_;
    std::map<std::string, std::string> unique_rdf_prefix_definitions_;
    std::vector<TripleNodes> rdf_triples_definitions_;

    SerdSyntax getSerdSyntax(const ReasonerSyntaxType& format);
    void addSuportedPrefixes(const std::string& prefixes);
    void addTriplePrefix(std::string& prefix);
    std::string createInstanceUri(const std::string& prefix, const std::string& name);
    std::tuple<std::string, std::string> extractTupleFromString(std::regex pattern,
                                                                std::string value);

    std::tuple<std::string, std::string> extractPrefixAndIdentifierFromRdfElement(
        const std::string& element);
};

#endif  // TRIPLE_WRITER_H
