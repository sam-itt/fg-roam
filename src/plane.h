#ifndef PLANE_H
#define PLANE_H

#include <stdbool.h>
#include <GL/glut.h>
#include <cglm/cglm.h>

#include "geo-location.h"

#include "gps-feed.h"
#include "gps-file-feed.h"

typedef struct{
    GeoLocation geopos;
#if 0
    double lat; /*degrees*/
    double lon; /*degrees*/
#endif
    double alt; /*meters*/

    /*degrees*/
    double roll;
    double pitch;
    double heading;

    /*ECEF coordinates (meters)*/
    double X;
    double Y;
    double Z;

    float speed; /*m/s*/

    GLfloat vroll;
    GLfloat vpitch;
    GLfloat vheading;

    GLfloat vX;
    GLfloat vY;
    GLfloat vZ;

    vec3 x, y, z;
    bool dirty;

    mat4d attitude;
    mat4d view;
} Plane;


Plane *plane_new(void);
void plane_free(Plane *self);

void plane_view(Plane *self);
void DumpPlane(Plane *p);

void plane_get_position(Plane *p, double *lat, double *lon, double *alt);
void plane_set_position(Plane *self, double lat, double lon, double alt);
void plane_set_attitude(Plane *p, double roll, double pitch, double heading);
void plane_show_position(Plane *p);
void plane_update(Plane *self, GpsFeed *feed);

void plane_update_position(Plane *self, double lat, double lon, double alt, time_t dt);
void plane_update_timed(Plane *self, GpsFileFeed *feed, double dt);
void plane_update_position2(Plane *self, time_t dt);
#endif
