#define OBSERVATION_ID_UTILS_H
#ifdef OBSERVATION_ID_UTILS_H

#include <chrono>
#include <string>

class ObservationIdentifier {
   public:
    static const std::string createObservationIdentifier(
        const std::chrono::system_clock::time_point& timestamp);
};

#endif  // OBSERVATION_ID_UTILS_H