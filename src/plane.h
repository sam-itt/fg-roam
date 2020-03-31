#ifndef PLANE_H
#define PLANE_H

#include <stdbool.h>
#include <GL/glut.h>
#include <cglm/cglm.h>


typedef struct{
    double lat; /*degrees*/
    double lon; /*degrees*/
    float alt; /*meters*/

    /*degrees*/
    GLfloat roll; 
    GLfloat pitch; 
    GLfloat heading;

    /*ECEF coordinates (meters)*/
    GLfloat X;
    GLfloat Y;
    GLfloat Z;

    float speed; /*m/s*/

    GLfloat vroll; 
    GLfloat vpitch; 
    GLfloat vheading;

    GLfloat vX; 
    GLfloat vY; 
    GLfloat vZ;

    vec3 n, e, d;
    vec3 x, y, z;
    bool inited;

    mat4 attitude;
    mat4 view;
} Plane;


Plane *plane_new(void);
void plane_free(Plane *self);

void PlaneView(Plane *p, double dt);
void DumpPlane(Plane *p);

void plane_get_position(Plane *p, double *lat, double *lon, double *alt);
void plane_set_position(Plane *self, double lat, double lon, double alt);
void plane_show_position(Plane *p);
#endif
