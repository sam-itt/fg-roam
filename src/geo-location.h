/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef GEO_LOCATION_H
#define GEO_LOCATION_H

#include <math.h>
#include <stdbool.h>

typedef struct{
    /*Degrees*/
    double latitude;
    double longitude;
}GeoLocation;

#define deg2rad(deg) ((deg) * M_PI/180.0)
#define rad2deg(rad) ((rad) * 180.0/M_PI)

bool geo_location_set(GeoLocation *self, double latitude, double longitude);
double geo_location_distance_to(GeoLocation *self, GeoLocation *location);
bool geo_location_bounding_coordinates(GeoLocation *self, double distance, GeoLocation bounds[2]);

static inline bool geo_location_set_rad(GeoLocation *self, double latitude, double longitude)
{
    return geo_location_set(self, rad2deg(latitude), rad2deg(longitude));
}

#endif /* GEO_LOCATION_H */
