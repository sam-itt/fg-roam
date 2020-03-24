#ifndef PLANE_H
#define PLANE_H

#include <stdbool.h>
#include <GL/glut.h>

#include <cglm/cglm.h>

typedef struct{
    GLfloat X; 
    GLfloat Y; 
    GLfloat Z;

    GLfloat roll; 
    GLfloat pitch; 
    GLfloat yaw;

    double bearing;
    bool dirty; 

    GLfloat vroll; 
    GLfloat vpitch; 
    GLfloat vyaw;

    GLfloat vX; 
    GLfloat vY; 
    GLfloat vZ;

    mat4 view;
} Plane;

void PlaneView(Plane *p);
void DumpPlane(Plane *p);

void plane_get_position(Plane *p, double *lat, double *lon, double *alt);
void plane_show_position(Plane *p);
#endif
