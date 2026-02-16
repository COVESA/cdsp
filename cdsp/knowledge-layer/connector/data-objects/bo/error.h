#ifndef ERROR_H
#define ERROR_H

#include <nlohmann/json.hpp>
#include <optional>
#include <ostream>
#include <string>

class Error {
   public:
    Error(int code, std::string message, std::optional<nlohmann::json> data);

    [[nodiscard]] int getCode() const;
    [[nodiscard]] const std::string &getMessage() const;
    [[nodiscard]] const std::optional<nlohmann::json> &getData() const;
    friend std::ostream &operator<<(std::ostream &out_stream, const Error &error);

   private:
    int code_;
    std::string message_;
    std::optional<nlohmann::json> data_;
};

#endif  // ERROR_H