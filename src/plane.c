/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "plane.h"
#include "gps-feed.h"
#include "gps-file-feed.h"

#include "geodesy.h"
#include "quat-ext.h"

#include "sg_geod.h"


Plane *plane_new(void)
{
    Plane *rv;

    rv = calloc(1, sizeof(Plane));
    if(rv){
        rv->X = NAN;
        rv->Y = NAN;
        rv->Z = NAN;

        glm_mat4d_identity(rv->attitude);
    }
    return rv;
}

void plane_free(Plane *self)
{
    free(self);
}

void plane_view(Plane *self)
{
    if(!self->dirty)
        return;

    if(self->speed){
        self->X += self->x[0] * self->speed;
        self->Y += self->x[1] * self->speed;
        self->Z += self->x[2] * self->speed;
    }

    self->X += self->vX;
    self->Y += self->vY;
    self->Z += self->vZ;

    self->roll += self->vroll;
    self->pitch += self->vpitch;
    self->heading += self->vheading;

#if 0
    fmod(self->roll, 360);
    fmod(self->pitch, 360);
    fmod(self->heading, 360);
#endif
    glm_mat4d_identity(self->view);

    versord hlOr;
    versord hlToBody;
    versord ec2body;
    versord q;
    versord mViewOrientation;

   // The quaternion rotating from the earth centered frame to the
    // horizontal local frame
    glm_quatd_from_lon_lat(hlOr, self->geopos.longitude, self->geopos.latitude);

    // The rotation from the horizontal local frame to the basic view orientation
    glm_quatd_from_ypr(hlToBody, self->heading, self->pitch, self->roll);

    // Compute the eyepoints orientation and position
    // wrt the earth centered frame - that is global coorinates
    glm_quatd_mul(hlOr, hlToBody, ec2body);

    // The cartesian position of the basic view coordinate
    vec3d position = {self->X, self->Y, self->Z};

    // This is rotates the x-forward, y-right, z-down coordinate system the where
    // simulation runs into the OpenGL camera system with x-right, y-up, z-back.
    glm_quatd_init(q, -0.5, -0.5, 0.5, 0.5);

//    self->_absolute_view_pos = position;
    glm_quatd_mul(ec2body, q, mViewOrientation);

    mat4d rotation;
    versord view_inv;
    glm_quatd_inv(mViewOrientation, view_inv);
    glm_quatd_mat4d(view_inv, rotation);

//    printf("%f,%f,%f -> %f %f %f %f\n",self->heading, self->pitch, self->roll, view_inv[0],view_inv[1],view_inv[2],view_inv[3]);

    glm_mat4d_mul(self->view, rotation, self->view);
    glm_translated(self->view, (vec3d){-self->X, -self->Y, -self->Z});
    self->dirty = false;
}




void plane_set_attitude(Plane *p, double roll, double pitch, double heading)
{
    p->roll = roll;
    p->pitch = pitch;
    p->heading = heading;

    p->dirty = true;
}

/*
 * Alt in meters
 *
 */
void plane_set_position(Plane *self, double lat, double lon, double alt)
{
    self->geopos.latitude = lat;
    self->geopos.longitude = lon;
    self->alt = alt;

    SGGeodToCart(self->geopos.latitude, self->geopos.longitude, self->alt, &self->X, &self->Y, &self->Z);

    self->dirty = true;
}


void plane_get_position(Plane *p, double *lat, double *lon, double *alt)
{
    /*TODO: just dirty out position on change using setters*/
    static double old[3] = {NAN, NAN, NAN};
    static double oldrv[3];

    if((p->X != old[0]) || (p->Y != old[1]) || (p->Z != old[2])){
        const double *values;
        double xvec[3];

        xvec[0] = p->X/1000.0;
        xvec[1] = p->Y/1000.0;
        xvec[2] = p->Z/1000.0;

        values = xyzllh(xvec);

        p->heading = geo_bearing(oldrv[0], oldrv[1], values[0], values[1]);

        oldrv[0] = values[0];
        oldrv[1] = values[1];
        oldrv[2] = values[2];

        old[0] = p->X;
        old[1] = p->Y;
        old[2] = p->Z;

    }
    *lat = oldrv[0];
    *lon = oldrv[1];
    *alt = oldrv[2];
}



/*dt  in seconds*/
void plane_update_position(Plane *self, double lat, double lon, double alt, time_t dt)
{
    double oldX, oldY, oldZ;

    oldX = self->X;
    oldY = self->Y;
    oldZ = self->Z;

//    printf("Plane prev pos (X,Y,Z): %0.5f, %0.5f, %0.5f\n",oldX,oldY,oldZ);
    plane_set_position(self, lat, lon, alt);
    if(!isnan(oldX)){
        double dX, dY, dZ;

//        printf("Plane new pos (X,Y,Z): %0.5f, %0.5f, %0.5f\n",self->X,self->Y,self->Z);
        dX = self->X - oldX;
        dY = self->Y - oldY;
        dZ = self->Z - oldZ;

//        printf("Plane movement (dX,dY,dZ): %0.5f, %0.5f, %0.5f\n",dX,dY,dZ);
        self->vX = dX/dt;
        self->vY = dY/dt;
        self->vZ = dZ/dt;

//        printf("Plane speeds (X,Y,Z): %0.5f, %0.5f, %0.5f\n",self->vX,self->vY,self->vZ);
    }
}

/*dt  in seconds*/
void plane_update_position2(Plane *self, time_t dt)
{
    float distance;

    distance = self->speed * dt; /*m/s * s -> m*/
    double distRatio = distance / EARTH_RADIUS*1000.0;
    double distRatioSine = sin(distRatio);
    double distRatioCosine = cos(distRatio);

    double startLatRad = glm_rad(self->geopos.latitude);
    double startLonRad = glm_rad(self->geopos.longitude);

    double startLatCos = cos(startLatRad);
    double startLatSin = sin(startLatRad);

    double endLatRads = asin((startLatSin * distRatioCosine) + (startLatCos * distRatioSine * cos(glm_rad(self->heading))));

    double endLonRads = startLonRad
        + atan2(sin(glm_rad(self->heading)) * distRatioSine * startLatCos,
            distRatioCosine - startLatSin * sin(endLatRads));

    self->geopos.latitude = glm_deg(endLatRads);
    self->geopos.longitude = glm_deg(endLonRads);

    self->alt = self->alt + (sinf(glm_rad(self->pitch)) * (self->speed*dt));

}




void plane_update(Plane *self, GpsFeed *feed)
{
    GpsRecord rec;
    static GpsRecord last_rec = GPS_RECORD_INVALID;

    gps_feed_get_next(feed, &rec);
    if(!gps_record_equals(&rec, &last_rec)){
        plane_set_position(self, rec.lat, rec.lon, rec.alt);
//        plane_update_position(self, rec.lat, rec.lon, rec.alt, rec.time - last_rec.time);
        last_rec = rec;
    }
}

void plane_update_timed(Plane *self, GpsFileFeed *feed, double dt)
{
    GpsRecord rec;

    gps_file_feed_get(feed, &rec, dt);
    plane_set_position(self, rec.lat, rec.lon, rec.alt);
}

void plane_show_position(Plane *p)
{
    double lat, lon, alt;

    plane_get_position(p, &lat, &lon, &alt);
    printf("Current position: lat: %0.5f, lng: %0.5f, altitude: %0.2f ft\n", lat, lon, (alt*1000.0)*3.28084);
}

void DumpPlane(Plane *p)
{
    printf("Plane position(X,Y,Z): %0.5f, %0.5f, %0.5f\n",p->X,p->Y,p->Z);
    printf("Plane attiude: roll:%0.5f pitch: %0.5f heading: %0.5f\n",p->roll,p->pitch,p->heading);
    printf("XYZ vectors:\n");
    printf("x: "); vec3_dump(p->x);
    printf("y: "); vec3_dump(p->y);
    printf("z: "); vec3_dump(p->z);

#if 0
    printf("Plane View Matrix:\n");
    mat4_dump(p->view);
#endif


}
