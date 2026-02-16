#ifndef DATA_HEADER_H
#define DATA_HEADER_H

#include <string>

#include "data_types.h"

class MessageHeader {
   public:
    MessageHeader(const std::string &instance, const SchemaType &schema_type);

    [[nodiscard]] std::string getInstance() const;
    [[nodiscard]] SchemaType getSchemaType() const;

   private:
    std::string instance_;
    SchemaType schema_type_;
};

#endif  // DATA_HEADER_H