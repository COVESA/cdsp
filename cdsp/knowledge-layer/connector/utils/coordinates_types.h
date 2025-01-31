#ifndef COORDINATES_TYPES_H
#define COORDINATES_TYPES_H

#include <algorithm>
#include <cinttypes>
#include <tuple>

struct Wgs84Coord {
    double longitude{0.0};
    double latitude{0.0};
    double altitude{0.0};

    friend bool operator==(const Wgs84Coord& lhs, const Wgs84Coord& rhs) noexcept {
        return std::tie(lhs.longitude, lhs.latitude, lhs.altitude) ==
               std::tie(rhs.longitude, rhs.latitude, rhs.altitude);
    }
};

struct NtmCoord {
    double easting{0.0};  ///< Easting in meters. Value range with guaranteed precision: 0..1000000.
    double northing{
        0.0};  ///< Northing in meters. Value range with guaranteed precision: 0..9600000.
    double altitude{0.0};             ///< Altitude in meters
    std::uint32_t projection_id{0U};  ///< Associated projection id

    friend bool operator==(const NtmCoord& lhs, const NtmCoord& rhs) noexcept {
        return std::tie(lhs.easting, lhs.northing, lhs.altitude, lhs.projection_id) ==
               std::tie(rhs.easting, rhs.northing, rhs.altitude, rhs.projection_id);
    }

    friend bool operator!=(const NtmCoord& lhs, const NtmCoord& rhs) noexcept {
        return !(lhs == rhs);
    }
};

#endif  // COORDINATES_TYPES_H
