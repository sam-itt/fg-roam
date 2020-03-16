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
} Plane;

void PlaneView(Plane *p);
void DumpPlane(Plane *p);

#endif
