/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <math.h>
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

/**
 * @brief Converts a decimal latitude to a DMS expression.
 *
 * @param latitude The latitude to convert
 * @param obuf Where to write the output string. Must be at least 14 bytes
 * long. i.e. This function will write 13 chars and a NULL bytes at this
 * location.
 *
 * @return @param obuf or NULL on failure
 */
char *geo_location_latitude_to_dms(double latitude, char *obuf)
{
    /* Return format:
     * L999°99.99' => 13 chars, 14 including trailing \0
     * */

    obuf[0] = latitude >= 0 ? 'N' : 'S';
    return geo_location_coordinate_to_dms(latitude, obuf+1);
}

/**
 * @brief Converts a decimal longitude to a DMS expression.
 *
 * @param longitude The longitude to convert
 * @param obuf Where to write the output string. Must be at least 14 bytes
 * long. i.e. This function will write 13 chars and a NULL bytes at this
 * location.
 *
 * @return @param obuf or NULL on failure
 */
char *geo_location_longitude_to_dms(double longitude, char *obuf)
{
    /* Return format:
     * L999°99.99' => 13 chars, 14 including trailing \0
     * */

    obuf[0] = longitude >= 0 ? 'E' : 'W';
    return geo_location_coordinate_to_dms(longitude, obuf+1);
}


/**
 * @brief Converts a decimal coordinate to a DMS string representation.
 *
 * @param coordinate The coordinate to convert
 * @param obuf Where to write the output string. Must be at least 13 bytes long
 * i.e. This function will write at max 12 chars and a NULL bytes at this
 * location.
 *
 * @return @param obuf or NULL on failure
 */
char *geo_location_coordinate_to_dms(double coordinate, char *obuf)
{
    /* Return format:
     * 999°99.99' => 12 chars, 13 including trailing \0
     * */
    double absolute = fabs(coordinate);
    int degree = floor(absolute);
    double fullMinutes = (absolute-degree) * 60.0;
    int minutes = floor(fullMinutes);
    int seconds = floor((fullMinutes-minutes)*60.0);

    sprintf(obuf, "%3d\x8f%02d.%02d'", degree, minutes, seconds);
    return obuf;
}

/**
 * @brief Compute the bearing in degrees from @param self to @param dest
 *
 * @param self A GeoLocation
 * @param dest Another GeoLocation
 *
 * @return The bearing between the two, in degrees.
 */
double geo_location_bearing(GeoLocation *self, GeoLocation *dest)
{
    double delta_lon;
    double slat = deg2rad(self->latitude);
    double slon = deg2rad(self->longitude);
    double dlat = deg2rad(dest->latitude);
    double dlon = deg2rad(dest->longitude);

    delta_lon = dlon - slon;

    double x = cos(dlat)*sin(delta_lon);
    double y = cos(slat)*sin(dlat) - sin(slat)*cos(dlat)*cos(delta_lon);

    double rv = rad2deg(atan2(x,y));
    if(rv < 0)
        rv += 360;
    return rv;
}

static inline bool valid_latitude(double value)
{
    return value >= MIN_LAT && value <= MAX_LAT;
}

static inline bool valid_longitude(double value)
{
    return value >= MIN_LON && value <= MAX_LON;
}
