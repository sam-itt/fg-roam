/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include "geo-location.h"

#define EARTH_RADIUS_M 6371009

#define MIN_LAT -90.0
#define MAX_LAT 90.0
#define MIN_LON -180.0
#define MAX_LON 180.0

#define MIN_LAT_RAD deg2rad(MIN_LAT) /* -PI/2 */
#define MAX_LAT_RAD deg2rad(MAX_LAT) /* PI/2 */
#define MIN_LON_RAD deg2rad(MIN_LON) /* -PI */
#define MAX_LON_RAD deg2rad(MAX_LAT) /* -PI */

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

static inline bool valid_latitude(double value);
static inline bool valid_longitude(double value);

/**
 * @brief Sets GeoLocation from lat/long values in degrees.
 *
 * This function will check the given values and fail if they
 * are out of range.
 *
 * @param self a GeoLocation
 * @param latitude latitude in degrees
 * @param longitude longitude in degrees
 * @return true on success, false on failure
 *
 * @see geo_location_set_rad
 */
bool geo_location_set(GeoLocation *self, double latitude, double longitude)
{
    if(!valid_latitude(latitude) || !valid_longitude(longitude))
        return false;

    self->latitude = latitude;
    self->longitude = longitude;

    return true;
}

/**
* @brief Computes shortest (great circle) between @p self and the given
* location.
*
* @param self a GeoLocation
* @param location the location to compute distance to
* @return the distance, in meters.
*/
double geo_location_distance_to(GeoLocation *self, GeoLocation *location)
{

    return acos(
        sin(deg2rad(self->latitude)) * sin(deg2rad(location->latitude)) +
        cos(deg2rad(self->latitude)) * cos(deg2rad(location->latitude)) *
        cos(deg2rad(self->longitude) - deg2rad(location->longitude))
    ) * EARTH_RADIUS_M;
}


/**
* @brief Computes bounding coordinates of @p distance around @pself
*
* For more information about the formulae used in this method visit
* <a href="http://JanMatuschek.de/LatitudeLongitudeBoundingCoordinates">
* http://JanMatuschek.de/LatitudeLongitudeBoundingCoordinates</a>.
*
* @param self a GeoLocation
* @param distance the distance in meters from the point represented by @p self.
* @param bounds an array of two GeoLocation objects that will be filled such
* that:<ul>
* <li>The latitude of any point within the specified distance is greater
* or equal to the latitude of the first array element and smaller or
* equal to the latitude of the second array element.</li>
* <li>If the longitude of the first array element is smaller or equal to
* the longitude of the second element, then
* the longitude of any point within the specified distance is greater
* or equal to the longitude of the first array element and smaller or
* equal to the longitude of the second array element.</li>
* <li>If the longitude of the first array element is greater than the
* longitude of the second element (this is the case if the 180th
* meridian is within the distance), then
* the longitude of any point within the specified distance is greater
* or equal to the longitude of the first array element
* <strong>or</strong> smaller or equal to the longitude of the second
* array element.</li>
* </ul>
* @return true on success, false on failure
*/
bool geo_location_bounding_coordinates(GeoLocation *self, double distance, GeoLocation bounds[2])
{
    if(distance < 0) return false;

    // angular distance in radians on a great circle
    double radDist = distance / EARTH_RADIUS_M;
    double radLat, radLon;

    radLat = deg2rad(self->latitude);
    radLon = deg2rad(self->longitude);

    double minLat = radLat - radDist;
    double maxLat = radLat + radDist;

    double minLon, maxLon;
    if (minLat > MIN_LAT && maxLat < MAX_LAT) {
        double deltaLon = asin(
            sin(radDist) / cos(radLat)
        );
        minLon = radLon - deltaLon;
        if (minLon < MIN_LON) minLon += 2.0 * M_PI;
        maxLon = radLon + deltaLon;
        if (maxLon > MAX_LON) maxLon -= 2.0 * M_PI;
    } else {
        // a pole is within the distance
        minLat = MAX(minLat, MIN_LAT);
        maxLat = MIN(maxLat, MAX_LAT);
        minLon = MIN_LON;
        maxLon = MAX_LON;
    }

    geo_location_set_rad(&bounds[0], minLat, minLon);
    geo_location_set_rad(&bounds[1], maxLat, maxLon);
    return true;
}


static inline bool valid_latitude(double value)
{
    return value >= MIN_LAT && value <= MAX_LAT;
}

static inline bool valid_longitude(double value)
{
    return value >= MIN_LON && value <= MAX_LON;
}
