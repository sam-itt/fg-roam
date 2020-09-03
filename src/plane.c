#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "plane.h"
#include "gps-feed.h"
#include "gps-file-feed.h"

#include "geodesy.h"

Plane *plane_new(void)
{
    Plane *rv;

    rv = calloc(1, sizeof(Plane));
    if(rv){
        rv->X = NAN;
        rv->Y = NAN;
        rv->Z = NAN;
        rv->n[0] = NAN;

        glm_mat4_identity(rv->attitude);
        /*Mickey-mouse axes to get a straight view*/
        glm_rotate(rv->attitude, glm_rad(90), (vec3){0.0f, 0.0f, 1.0f});
        glm_rotate(rv->attitude, glm_rad(45), (vec3){0.0f, 1.0f, 0.0f});

    }
    return rv;
}

void plane_free(Plane *self)
{
    free(self);
}

void PlaneView(Plane *p, double dt)
{
    mat3 mtmp;
    /*
    heading is pitch -> 45
    roll is roll
    pitch is heading*/

    dt = 1.0;

    if(p->speed){
        p->X += p->x[0] * p->speed * dt;
        p->Y += p->x[1] * p->speed * dt;
        p->Z += p->x[2] * p->speed * dt;
    }

    p->X += p->vX * dt;
    p->Y += p->vY * dt;
    p->Z += p->vZ * dt;

    p->roll += p->vroll;
    p->pitch += p->vpitch;
    p->heading += p->vheading;

    fmod(p->roll, 360);
    fmod(p->pitch, 360);
    fmod(p->heading, 360);

    glm_mat4_identity(p->view);
    if(!p->inited){
        glm_vec3_copy(p->n, p->x);
        glm_vec3_copy(p->e, p->y);
        glm_vec3_copy(p->d, p->z);

        glm_rotate(p->attitude, glm_rad(0), p->x);
        glm_rotate(p->attitude, glm_rad(180), p->z);
        glm_rotate(p->attitude, glm_rad(0), p->y);

        printf("Using local axes !\n");

        p->inited = true;
    }

    if(p->vheading != 0){
        glm_rotate(p->attitude, glm_rad(p->vheading), p->z);
        glm_mat3_zero(mtmp);
        geo_mat3_rot(p->z, -glm_rad(p->vheading), mtmp);
        glm_mat3_mulv(mtmp, p->x, p->x);
        glm_mat3_mulv(mtmp, p->y, p->y);
        glm_mat3_mulv(mtmp, p->z, p->z);
    }
    if(p->vpitch != 0){
        glm_rotate(p->attitude, glm_rad(p->vpitch), p->y);
        glm_mat3_zero(mtmp);
        geo_mat3_rot(p->y, -glm_rad(p->vpitch), mtmp);
        glm_mat3_mulv(mtmp, p->x, p->x);
        glm_mat3_mulv(mtmp, p->y, p->y);
        glm_mat3_mulv(mtmp, p->z, p->z);
    }
    if(p->vroll != 0){
        glm_rotate(p->attitude, glm_rad(p->vroll), p->x);
        glm_mat3_zero(mtmp);
        geo_mat3_rot(p->x, -glm_rad(p->vroll), mtmp);
        glm_mat3_mulv(mtmp, p->x, p->x);
        glm_mat3_mulv(mtmp, p->y, p->y);
        glm_mat3_mulv(mtmp, p->z, p->z);
    }

    glm_mat4_mul(p->view, p->attitude, p->view);
    glm_translate(p->view, (vec3){-p->X, -p->Y, -p->Z});

}

void plane_set_attitude(Plane *p, double roll, double pitch, double heading)
{
    mat3 mtmp;

    p->roll = roll;
    p->pitch = pitch;
    p->heading = heading;

    glm_mat4_identity(p->attitude);
    /*Mickey-mouse axes to get a straight view*/
    glm_rotate(p->attitude, glm_rad(86), (vec3){0.0f, 0.0f, 1.0f});
    glm_rotate(p->attitude, glm_rad(40), (vec3){0.0f, 1.0f, 0.0f});

    glm_vec3_copy(p->n, p->x);
    glm_vec3_copy(p->e, p->y);
    glm_vec3_copy(p->d, p->z);

    glm_rotate(p->attitude, glm_rad(0), p->x);
    glm_rotate(p->attitude, glm_rad(180), p->z);
    glm_rotate(p->attitude, glm_rad(0), p->y);

    glm_rotate(p->attitude, -glm_rad(p->heading), p->z);
    glm_mat3_zero(mtmp);
    geo_mat3_rot(p->z, -glm_rad(p->heading), mtmp);
    glm_mat3_mulv(mtmp, p->x, p->x);
    glm_mat3_mulv(mtmp, p->y, p->y);
    glm_mat3_mulv(mtmp, p->z, p->z);

    glm_rotate(p->attitude, glm_rad(p->pitch), p->y);
    glm_mat3_zero(mtmp);
    geo_mat3_rot(p->y, glm_rad(p->pitch), mtmp);
    glm_mat3_mulv(mtmp, p->x, p->x);
    glm_mat3_mulv(mtmp, p->y, p->y);
    glm_mat3_mulv(mtmp, p->z, p->z);

    glm_rotate(p->attitude, glm_rad(p->roll), p->x);
    glm_mat3_zero(mtmp);
    geo_mat3_rot(p->x, glm_rad(p->roll), mtmp);
    glm_mat3_mulv(mtmp, p->x, p->x);
    glm_mat3_mulv(mtmp, p->y, p->y);
    glm_mat3_mulv(mtmp, p->z, p->z);

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

/*
 * Alt in meters
 *
 */
void plane_set_position(Plane *self, double lat, double lon, double alt)
{
    const double *pos;

    self->lat = lat;
    self->lon = lon;
    self->alt = alt;

    geo_get_ned(self->lat, self->lon, self->n, self->e, self->d);

    pos = llhxyz(lat, lon, alt/1000.0); /*rv in KM*/
    self->X = pos[0]*1000.0;
    self->Y = pos[1]*1000.0;
    self->Z = pos[2]*1000.0;
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

    double startLatRad = glm_rad(self->lat);
    double startLonRad = glm_rad(self->lon);

    double startLatCos = cos(startLatRad);
    double startLatSin = sin(startLatRad);

    double endLatRads = asin((startLatSin * distRatioCosine) + (startLatCos * distRatioSine * cos(glm_rad(self->heading))));

    double endLonRads = startLonRad
        + atan2(sin(glm_rad(self->heading)) * distRatioSine * startLatCos,
            distRatioCosine - startLatSin * sin(endLatRads));

    self->lat = glm_deg(endLatRads);
    self->lon = glm_deg(endLonRads);

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
    printf("NED vectors:\n");
    printf("n: "); vec3_dump(p->n);
    printf("e: "); vec3_dump(p->e);
    printf("d: "); vec3_dump(p->d);
    printf("XYZ vectors:\n");
    printf("x: "); vec3_dump(p->x);
    printf("y: "); vec3_dump(p->y);
    printf("z: "); vec3_dump(p->z);

#if 0
    printf("Plane View Matrix:\n");
    printf("vectors:\n");
    printf("n: "); vec3_dump(p->n);
    printf("e: "); vec3_dump(p->e);
    printf("d: "); vec3_dump(p->d);
    mat4_dump(p->view);
#endif


}
