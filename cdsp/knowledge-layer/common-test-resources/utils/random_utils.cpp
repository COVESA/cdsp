#include "random_utils.h"

#include <random>
/**
 * @brief Generates a random float of a specified range.
 */
float RandomUtils::generateRandomFloat(float min, float max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

/**
 * @brief Generates a random integer of a specified range.
 */
int RandomUtils::generateRandomInt(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

int64_t RandomUtils::generateRandomInt64(int64_t min, int64_t max) {
    std::random_device rd;
    std::mt19937_64 gen(rd());  // Use 64-bit Mersenne Twister
    std::uniform_int_distribution<int64_t> dis(min, max);
    return dis(gen);
}

/**
 * @brief Generates a random string of a specified length.
 */
std::string RandomUtils::generateRandomString(std::optional<size_t> length) {
    if (!length.has_value()) {
        length = generateRandomInt(1, 10);
    }
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    std::string result;
    result.reserve(length.value());
    for (size_t i = 0; i < result.capacity(); ++i) {
        result += charset[dis(gen)];
    }
    return result;
}

/**
 * @brief Generates a random double within a specified range.
 */
double RandomUtils::generateRandomDouble(double min, double max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(min, max);
    return dis(gen);
}

/**
 * @brief Generates a random value of a random type.
 */
std::variant<std::string, int, double, float, bool> RandomUtils::generateRandomValue() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> type_dist(0, 4);
    int type = type_dist(rng);

    switch (type) {
        case 0:
            return generateRandomString(8);  // String
        case 1:
            return generateRandomInt(-100, 100);  // Integer
        case 2:
            return generateRandomDouble(-1000, 1000);  // Double
        case 3:
            return generateRandomFloat(-1000, 1000);  // Float
        case 4:
            return generateRandomBool();  // Boolean
        default:
            return "error";  // Should never reach here
    }
}

/**
 * @brief Generates a random boolean value.
 */
bool RandomUtils::generateRandomBool() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::bernoulli_distribution d(0.5);  // 50% true, 50% false
    return d(gen);
}

/**
 * @brief Generates a random UTC date as an ISO 8601 string with fractional seconds.
 *
 * @param start_year The earliest possible year for the random date.
 * @param end_year The latest possible year for the random date.
 * @return A random UTC date in ISO 8601 format with fractional seconds.
 */
std::chrono::system_clock::time_point RandomUtils::generateRandomTimestamp(int start_year,
                                                                           int end_year,
                                                                           bool includeNanos) {
    // Convert years to time_t
    std::tm start_tm = {0, 0, 0, 1, 0, start_year - 1900};  // Jan 1, start_year
    std::tm end_tm = {0, 0, 0, 31, 11, end_year - 1900};    // Dec 31, end_year

    std::time_t start_time = std::mktime(&start_tm);
    std::time_t end_time = std::mktime(&end_tm);

    // Generate a random time_t value between start_time and end_time
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::time_t> dist(start_time, end_time);
    std::time_t random_time = dist(gen);

    // Convert to time_point
    if (includeNanos) {
        std::uniform_int_distribution<int> nanos_dist(0, 999999999);
        return std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            std::chrono::system_clock::from_time_t(random_time) +
            std::chrono::nanoseconds(nanos_dist(gen)));
    }
    return std::chrono::system_clock::from_time_t(random_time);
}