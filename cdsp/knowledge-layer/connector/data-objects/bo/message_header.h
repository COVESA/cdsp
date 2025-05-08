#ifndef DATA_HEADER_H
#define DATA_HEADER_H

#include <optional>
#include <string>

#include "data_types.h"

class MessageHeader {
   public:
    MessageHeader(const std::string& id, const SchemaType& schema_defintion);

    std::string getId() const;
    SchemaType getSchemaType() const;

   private:
    std::string id_;
    SchemaType schema_type_;
};

#endif  // DATA_HEADER_H