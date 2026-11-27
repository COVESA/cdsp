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

/**
 * @brief One RDF triple as Serd nodes, written by `generateTripleOutput`.
 */
struct TripleNodes {
    // Serd node type and text of the subject, for example (SERD_CURIE, `car:VehicleVIN123`)
    std::pair<SerdType, std::string> subject;
    // Serd node type and text of the predicate, for example (SERD_CURIE, `sosa:hasMember`)
    std::pair<SerdType, std::string> predicate;
    // Serd node type and text of the object: an IRI, a CURIE or a literal value
    std::pair<SerdType, std::string> object;
    // Serd node type and text of the datatype, only for a literal object
    std::optional<std::pair<SerdType, std::string>> datatype;
};

/**
 * @brief A member observation of a frame: its IRI and its value, which the frame node also
 * carries with the property of the member as predicate.
 */
struct FrameMember {
    // IRI of the member observation as CURIE, for example `car:fm_Angle_VIN123_<frame timestamp>`
    std::string uri;
    // Property of the member as CURIE, for example `car:Angle`
    std::string property;
    // Value of the member as text, for example `250`
    std::string value;
    // Datatype of the value as CURIE, for example `xsd:short`
    std::string datatype;
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
    virtual FrameMember addMemberDataToTriple(
        const std::string& prefixes,
        const std::tuple<std::string, std::string, std::string>& rdf_data_values,
        const std::string& value, const std::chrono::system_clock::time_point& dataTime,
        const std::chrono::system_clock::time_point& frame_time,
        const std::optional<double>& ntmValue = std::nullopt);
    virtual void addFrameToTriple(const std::string& feature_of_interest_element,
                                  const std::chrono::system_clock::time_point& frame_time,
                                  const std::vector<FrameMember>& members);

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

    std::string addObservationToTriple(
        const std::string& prefixes,
        const std::tuple<std::string, std::string, std::string>& rdf_data_values,
        const std::string& value, const std::chrono::system_clock::time_point& timestamp,
        const std::optional<double>& ntmValue,
        const std::optional<std::chrono::system_clock::time_point>& frame_time);
    static std::string createTimestampIdentifier(
        const std::chrono::system_clock::time_point& timestamp);
};

#endif  // TRIPLE_WRITER_H
