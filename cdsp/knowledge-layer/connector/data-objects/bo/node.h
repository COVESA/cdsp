#ifndef NODE_H
#define NODE_H

#include <optional>
#include <string>
#include <vector>

#include "metadata.h"

class Node {
   public:
    Node(const std::string& name, const std::optional<std::string>& value, const Metadata& metadata,
         const std::vector<std::string>& supported_data_points);

    std::string getName() const;
    std::optional<std::string> getValue() const;
    Metadata getMetadata() const;

   private:
    std::string name_;
    std::optional<std::string> value_;
    Metadata metadata_;
};

#endif  // NODE_H