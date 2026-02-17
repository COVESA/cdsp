#ifndef REQUEST_REGISTRY_H
#define REQUEST_REGISTRY_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>

struct RequestInfo {
    enum Type : std::uint8_t { SUBSCRIBE, UNSUBSCRIBE, GET, SET } type;
    std::string schema;
    std::string instance;
    std::optional<std::string> path;

    bool operator==(const RequestInfo &other) const {
        return type == other.type && schema == other.schema && instance == other.instance &&
               path == other.path;
    }

    static const char *typeToString(Type type) {
        switch (type) {
            case Type::SUBSCRIBE:
                return "SUBSCRIBE";
            case Type::UNSUBSCRIBE:
                return "UNSUBSCRIBE";
            case Type::GET:
                return "GET";
            case Type::SET:
                return "SET";
            default:
                return "UNDEFINED";
        }
    }
};

class RequestRegistry {
   public:
    int addRequest(const RequestInfo &info);
    [[nodiscard]] std::optional<RequestInfo> getRequest(int identifier) const;
    [[nodiscard]] std::optional<int> findRequestId(const RequestInfo &info) const;
    void removeRequest(int identifier);

   private:
    std::map<int, RequestInfo> requests_;
    int next_identifier_ = 0;
};

#endif  // REQUEST_REGISTRY_H