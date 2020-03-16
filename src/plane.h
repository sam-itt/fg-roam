#ifndef PLANE_H
#define PLANE_H


#include <GL/glut.h>

typedef struct{
    GLfloat X; 
    GLfloat Y; 
    GLfloat Z;

    GLfloat roll; 
    GLfloat pitch; 
    GLfloat yaw;

    GLfloat vroll; 
    GLfloat vpitch; 
    GLfloat vyaw;

    GLfloat vX; 
    GLfloat vY; 
    GLfloat vZ;
} Plane;

void PlaneView(Plane *p);
void DumpPlane(Plane *p);

#endif
