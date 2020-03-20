#ifndef GEODESY_H
#define GEODESY_H

#define EARTH_RADIUS 6371

const double *xyzllh(double *xvec);
const double *llhxyz(double flat, double flon, double altkm);

double geo_dist(double lat1, double lon1, double lat2, double lon2);
double geo_bearing(double lat1, double lon1, double lat2, double lon2);
double *geo_destination(double olat, double olon, double distance, double bearing);
double *geo_bounding_box(double clat, double clon, double radius);
#endif
