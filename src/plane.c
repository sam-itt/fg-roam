#include <stdio.h>
#include <stdlib.h>
#include "plane.h"

void PlaneView(Plane *p)
{
    p->X += p->vX;
    p->Y += p->vY;
    p->Z += p->vZ;

    p->roll += p->vroll;
    p->pitch += p->vpitch;
    p->yaw += p->vyaw;

    // roll is rotation about the z axis 
    glRotatef(p->roll, 0.0f, 0.0f, 1.0f);
    // yaw, or heading, is rotation about the y axis 
    glRotatef(p->yaw, 0.0f, 1.0f, 0.0f);
    // pitch is rotation about the x axis 
    glRotatef(p->pitch, 1.0f, 0.0f, 0.0f);
    // move the plane to the plane’s world coordinates 
    glTranslatef(-p->X, -p->Y, -p->Z);
    // orientation
}


void DumpPlane(Plane *p)
{
    printf("Plane position(X,Y,Z): %0.5f, %0.5f, %0.5f\n",p->X,p->Y,p->Z);
    printf("Plane attiude: roll:%0.5f pitch: %0.5f yaw: %0.5f\n",p->roll,p->pitch,p->yaw);
}
