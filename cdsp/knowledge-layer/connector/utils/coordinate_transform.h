#ifndef COORDINATE_TRANSFORM_H
#define COORDINATE_TRANSFORM_H

#include <cmath>
#include <optional>

#include "coordinates_types.h"

constexpr double PI_IN_DEG = 180.0;
constexpr double COORDINATE_SCALLING = PI_IN_DEG / 2.0 / double(1ULL << 30U);
constexpr double MIN_VALID_LONGITUDE_IN_DEG = -PI_IN_DEG;
constexpr std::int32_t MAX_VALID_LONGITUDE_IN_NDS = std::numeric_limits<std::int32_t>::max();
constexpr double MIN_VALID_LATITUDE_IN_DEG = -90.0;
constexpr double MAX_VALID_LATITUDE_IN_DEG = 90.0;

namespace CoordinateTransform {
std::optional<NtmCoord> ntmPoseFromWgs84(const Wgs84Coord& zone_origin_wgs84,
                                         const Wgs84Coord& wgs84_coordinate);

};

#endif  // COORDINATE_TRANSFORM_H