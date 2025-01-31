#include "coordinate_transform.h"

#include <cmath>
#include <limits>

#include "GeographicLib/TransverseMercator.hpp"

namespace {
std::optional<Wgs84Coord> fromDeg(double longitude, double latitude) {
    // Validate longitude and latitude ranges
    if (!(longitude >= MIN_VALID_LONGITUDE_IN_DEG) ||
        !(std::trunc(longitude / COORDINATE_SCALLING) <=
          static_cast<double>(MAX_VALID_LONGITUDE_IN_NDS)) ||
        !(latitude >= MIN_VALID_LATITUDE_IN_DEG) || !(latitude <= MAX_VALID_LATITUDE_IN_DEG)) {
        return std::nullopt;
    }

    Wgs84Coord result;
    result.longitude = std::trunc(longitude / COORDINATE_SCALLING);
    result.latitude = std::trunc(latitude / COORDINATE_SCALLING);

    return result;
}

double inDeg(double lat_or_long) { return static_cast<double>(lat_or_long) * COORDINATE_SCALLING; }

double angleDifference(double angle1, double angle2) {
    // Calculate the difference between two angles
    double diff = angle2 - angle1;

    // Add or subtract 180 depending on the sign of the difference
    const auto offset = (diff > 0.0) ? 180.0 : -180.0;

    // Temporarily add offset
    diff += offset;

    // Normalize to the range [-180, 180]
    diff -= std::trunc(diff / 360.0) * 360.0;

    // Remove the offset and return the result
    return diff - offset;
}

bool isInOriginRange(const Wgs84Coord& origin, double longitude, double latitude) {
    // Ensure Transverse Mercator is not used beyond 75 degrees from the central meridian
    return (std::abs(angleDifference(inDeg(origin.longitude), longitude)) <= 75.0) &&
           (std::abs(latitude) <= 90.0);
}

double projectOriginNorthing(const GeographicLib::TransverseMercator& tmerc, double lat0,
                             double lon0) {
    double origin_northing = 0.0;
    double unused = 0.0;
    tmerc.Forward(lon0, lat0, lon0, unused, origin_northing);

    return origin_northing;
}

std::optional<NtmCoord> transformWgs84ToNtm(const Wgs84Coord& geo_coordinate,
                                            const Wgs84Coord& origin,
                                            GeographicLib::TransverseMercator& mercator) {
    const auto latitude = inDeg(geo_coordinate.latitude);
    const auto longitude = inDeg(geo_coordinate.longitude);

    if (!isInOriginRange(origin, longitude, latitude)) {
        return std::nullopt;
    }

    double easting = 0.0;
    double northing = 0.0;
    mercator.Forward(inDeg(origin.longitude), latitude, longitude, easting, northing);
    northing -= projectOriginNorthing(mercator, inDeg(origin.latitude), inDeg(origin.longitude));

    return NtmCoord{easting, northing, 0.0, 0};
}
}  // namespace

// Public functions
namespace CoordinateTransform {

/**
 * @brief Converts WGS84 coordinates to NTM coordinates using a specified zone origin.
 *
 * This function takes a WGS84 coordinate and a zone origin in WGS84 format, and converts
 * them to NTM (Norwegian Transverse Mercator) coordinates using the Transverse Mercator
 * projection. If the conversion is successful, it returns the NTM coordinates; otherwise,
 * it returns an empty optional.
 *
 * @param zone_origin_wgs84 The WGS84 coordinate representing the origin of the zone.
 * @param wgs84_coordinate The WGS84 coordinate to be converted to NTM.
 * @return std::optional<NtmCoord> The converted NTM coordinates, or std::nullopt if the
 * conversion fails.
 */
std::optional<NtmCoord> ntmPoseFromWgs84(const Wgs84Coord& zone_origin_wgs84,
                                         const Wgs84Coord& wgs84_coordinate) {
    // Create Transverse Mercator projection
    GeographicLib::TransverseMercator mercator{GeographicLib::Constants::WGS84_a(),
                                               GeographicLib::Constants::WGS84_f(), 1.0};

    // Convert the coordinates
    const auto origin = fromDeg(zone_origin_wgs84.longitude, zone_origin_wgs84.latitude);
    const auto coord = fromDeg(wgs84_coordinate.longitude, wgs84_coordinate.latitude);

    // Validate and transform
    if ((origin.has_value() && coord.has_value())) {
        const auto ntm_coordinate = transformWgs84ToNtm(coord.value(), origin.value(), mercator);
        return ntm_coordinate;
    }

    return std::nullopt;
}

}  // namespace CoordinateTransform