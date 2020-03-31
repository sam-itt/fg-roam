#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "plane.h"

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
