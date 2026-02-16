#ifndef STATUS_MESSAGE_H
#define STATUS_MESSAGE_H

#include <optional>
#include <ostream>

#include "error.h"

class StatusMessage {
   public:
    StatusMessage(int identifier, std::optional<Error> error);

    [[nodiscard]] int getIdentifier() const;
    [[nodiscard]] std::optional<Error> getError() const;
    friend std::ostream& operator<<(std::ostream& os, const StatusMessage& message);

   private:
    int id_;
    std::optional<Error> error_;
    std::chrono::system_clock::time_point timestamp_;
};

#endif  // STATUS_MESSAGE_H