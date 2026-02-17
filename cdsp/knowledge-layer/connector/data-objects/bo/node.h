#ifndef NODE_H
#define NODE_H

#include <optional>
#include <string>

#include "metadata.h"

class Node {
   public:
    Node(std::string name, std::optional<std::string> value, Metadata metadata);

    [[nodiscard]] std::string getName() const;
    [[nodiscard]] std::optional<std::string> getValue() const;
    [[nodiscard]] Metadata getMetadata() const;

   private:
    std::string name_;
    std::optional<std::string> value_;
    Metadata metadata_;
};

#endif  // NODE_H