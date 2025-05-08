#ifndef STATUS_MESSAGE_H
#define STATUS_MESSAGE_H

#include <chrono>
#include <optional>
#include <ostream>
#include <string>

class StatusMessage {
   public:
    StatusMessage(int code, const std::string& message, const std::optional<std::string>& requestId,
                  const std::chrono::system_clock::time_point& timestamp);

    int getCode() const;
    std::string getMessage() const;
    std::optional<std::string> getRequestId() const;
    std::chrono::system_clock::time_point getTimestamp() const;
    friend std::ostream& operator<<(std::ostream& os, const StatusMessage& message);

   private:
    int code_;
    std::string message_;
    std::optional<std::string> request_id_;
    std::chrono::system_clock::time_point timestamp_;
};

#endif  // STATUS_MESSAGE_H