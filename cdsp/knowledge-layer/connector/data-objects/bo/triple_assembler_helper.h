#ifndef TRIPLE_ASSEMBLER_HELPER_H
#define TRIPLE_ASSEMBLER_HELPER_H

#include <map>
#include <string>
#include <utility>

#include "data_types.h"

class TripleAssemblerHelper {
   public:
    struct QueryPair {
        std::pair<QueryLanguageType, std::string> data_property;
        std::pair<QueryLanguageType, std::string> object_property;
    };
    TripleAssemblerHelper(const std::map<SchemaType, QueryPair>& queries,
                          const std::string& output);
    std::map<SchemaType, QueryPair> getQueries() const;
    std::string getOutput() const;

   private:
    std::map<SchemaType, QueryPair> queries_;
    std::string output_;
};

#endif  // TRIPLE_ASSEMBLER_HELPER_H