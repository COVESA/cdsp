#define OBSERVATION_ID_UTILS_H
#ifdef OBSERVATION_ID_UTILS_H

#include <string>

class ObservationIdentifier {
   public:
    static const std::string createObservationIdentifier(const std::string& date_time,
                                                         const int identifier_counter);
};

#endif  // OBSERVATION_ID_UTILS_H