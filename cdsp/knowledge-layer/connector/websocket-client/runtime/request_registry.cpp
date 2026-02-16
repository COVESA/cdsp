#include "request_registry.h"

#include <iostream>
/**
 * @brief Adds a new request to the registry.
 *
 * This function takes a RequestInfo object and adds it to the
 * requests list associated with the next available identifier.
 * The identifier is then incremented for future requests.
 *
 * @param info The RequestInfo object containing details of the
 * request to be added.
 * @return The identifier of the newly added request.
 */
int RequestRegistry::addRequest(const RequestInfo &info) {
    requests_[next_identifier_] = info;
    return next_identifier_++;
}

/**
 * @brief Retrieves the request information associated with the given
 * identifier.
 *
 * This function searches for the request in the registry using the
 * provided identifier. If a request is found and it is not empty, the
 * first request info is returned. Otherwise, it returns an empty optional.
 *
 * @param identifier The unique identifier for the request to retrieve.
 * @return std::optional<RequestInfo> An optional containing the
 * request info if found, or std::nullopt if no request exists for the
 * given identifier.
 */
std::optional<RequestInfo> RequestRegistry::getRequest(int identifier) const {
    auto iterator = requests_.find(identifier);
    if (iterator != requests_.end()) {
        return iterator->second;
    }
    return std::nullopt;
}

/**
 * @brief Finds the request ID associated with the given request information.
 *
 * This method iterates through the registered requests and compares the
 * provided RequestInfo object with each request's information. If a match
 * is found based on type, schema, instance, and path, the corresponding
 * request ID is returned. If no match is found, -1 is returned to indicate
 * that the request ID could not be found.
 *
 * @param info The RequestInfo object containing the details to match against.
 * @return The identifier of the matching request if found; otherwise, -1.
 */
std::optional<int> RequestRegistry::findRequestId(const RequestInfo &info) const {
    for (const auto &[identifier, request_info] : requests_) {
        if (request_info == info) {
            return identifier;
        }
    }
    return std::nullopt;  // Not found
}

/**
 * @brief Removes a request from the registry using the given identifier.
 *
 * This function searches for the request in the registry using the
 * provided identifier. If found, it removes the request and returns
 * true. If not found, it returns false.
 *
 * @param identifier The unique identifier for the request to remove.
 * @return bool True if the request was successfully removed, false
 * otherwise.
 */
void RequestRegistry::removeRequest(int identifier) {
    auto iterator = requests_.find(identifier);
    if (iterator != requests_.end()) {
        RequestInfo request_info = iterator->second;

        requests_.erase(iterator);
    } else {
        std::cout << "Request with identifier " << identifier << " not found in the registry.\n";
    }
}
