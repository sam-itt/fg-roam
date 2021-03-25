/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef GEODESY_H
#define GEODESY_H

#include <cglm/cglm.h>

#define EARTH_RADIUS 6371

const double *xyzllh(double *xvec);
const double *llhxyz(double flat, double flon, double altkm);

double geo_dist(double lat1, double lon1, double lat2, double lon2);
double geo_bearing(double lat1, double lon1, double lat2, double lon2);
double *geo_destination(double olat, double olon, double distance, double bearing);


void geo_mat3_rot(vec3 n, float teta, mat3 result);
void geo_get_ned(double lat, double lon, vec3 n, vec3 e, vec3 d);
void geo_get_ned2(double lat, double lon, vec3 n0, vec3 e0, vec3 d0, vec3 n, vec3 e, vec3 d);
const double *geo_llh_to_dis(double lat, double lon, float alt, float heading, float pitch, float roll);


void vec3_dump(vec3 v);
void vec3_dump_long(vec3 v);
void mat3_dump(mat3 m);
void mat4_dump(mat4 m);
bool same_sign(float a, float b);
float local_round(float var);
#endif
