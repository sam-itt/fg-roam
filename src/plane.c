#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "plane.h"

#include "geodesy.h"

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

        p->bearing = geo_bearing(oldrv[0], oldrv[1], values[0], values[1]);

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


void plane_show_position(Plane *p)
{
    double lat, lon, alt;

    plane_get_position(p, &lat, &lon, &alt);
    printf("Current position: lat: %0.5f, lng: %0.5f, altitude: %0.2f ft\n", lat, lon, (alt*1000.0)*3.28084);
}

void DumpPlane(Plane *p)
{
    printf("Plane position(X,Y,Z): %0.5f, %0.5f, %0.5f\n",p->X,p->Y,p->Z);
    printf("Plane attiude: roll:%0.5f pitch: %0.5f yaw: %0.5f\n",p->roll,p->pitch,p->yaw);
}
