#include "triple_assembler_helper.h"

/**
 * @brief Constructs a TripleAssemblerHelper object with the given queries and output.
 *
 * This constructor initializes a TripleAssemblerHelper object with the provided queries to
 * construct the triples and the definition of the output file to put the triples.
 *
 * @param queries A map of SchemaType to QueryPair objects. Each QueryPair object contains
 *                a data property and an object property to construct the triples.
 * @throws std::invalid_argument if the output string is empty or if any of the queries are empty.
 */
TripleAssemblerHelper::TripleAssemblerHelper(const std::map<SchemaType, QueryPair>& queries)
    : queries_(queries) {
    for (const auto& query : queries_) {
        if (query.second.data_property.second.empty() ||
            query.second.object_property.second.empty()) {
            throw std::invalid_argument(
                "Queries `data_property` and `object_property` cannot be empty");
        }
    }
}

/**
 * @brief Retrieves the queries.
 *
 * This function returns the queries stored in the TripleAssemblerHelper object.
 *
 * @return A map of SchemaType to QueryPair objects. Each QueryPair object contains
 *                a data property and an object property to construct the triples.
 */
std::map<SchemaType, TripleAssemblerHelper::QueryPair> TripleAssemblerHelper::getQueries() const {
    return queries_;
}