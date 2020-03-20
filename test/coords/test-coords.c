#include <stdio.h>
#include <stdlib.h>

#include "geodesy.h"

int main(int argc, char *argv[])
{
    const double *rv;
    double in[3];
    double expected[3];
    int ev = EXIT_SUCCESS;

    //All KM
    in[0] =  4741.94757700; //X 
    in[1] =  185.75580481; //Y
    in[2] =  4247.92521818; //Z
    
    expected[0] = 0x1.50314177f8b77p+5;
    expected[1] = 0x1.1f2445c49c172p+1;
    expected[2] = 0x1.0161981d6ce98p-1;

    rv = xyzllh(in);
//    printf("rv is : lat:%f lng:%f height:%f\n", rv[0], rv[1], rv[2]);
    for(int i = 0; i < 3; i++){
        if(rv[i] != expected[i]){
           // printf("%s failure at %d, %a != %a\n","xyzllh",i,rv[i],expected[i]);
            ev = EXIT_FAILURE;
        }
    }

    double dist = geo_dist(36.12, -86.67, 33.94, -118.4);
    double expected_dist = 0x1.68ce38e02fc29p+11;
//    printf("dist is: %0.2f (%a)\n",dist,dist);
    if(dist != expected_dist)
        ev = EXIT_FAILURE;

    double pos[2][2] = {
        {42.02597, 2.23869},
        {41.99979, 2.23788}
    };

    dist = geo_dist(pos[0][0], pos[0][1], pos[1][0], pos[1][1]);
    double bearing = geo_bearing(pos[0][0], pos[0][1], pos[1][0], pos[1][1]);
    double expected_bearing = 0x1.6aa261f772fe2p+7;
/*    printf("Bearing between(%0.5f,%0.5f) and (%0.5f,%0.5f): %0.5f(%a)\n",
        pos[0][0], pos[0][1], 
        pos[1][0], pos[1][1],
        bearing,bearing
    );
    printf("dist is: %0.2f (%a)\n",dist,dist);*/
    if(bearing != expected_bearing)
        ev = EXIT_FAILURE;

// (41.99979(0x1.4fff91e646f16p+5),2.23788(0x1.1e72da122fad7p+1))

    double *dest = geo_destination(pos[0][0], pos[0][1], dist, bearing);
    double expected_dest[2] = {0x1.4fff91e646f16p+5,0x1.1e72da122fad7p+1};
/*    printf("From A(%0.5f,%0.5f) bearing %0.2f during %f km, landing at: (%0.5f(%a),%0.5f(%a))\n",
            43.102735, 3.112493, 315.11, 11.41,
            dest[0], dest[0],
            dest[1], dest[1]
    );*/
    if(dest[0] != expected_dest[0] || dest[1] != expected_dest[1])
        ev = EXIT_FAILURE;

	exit(ev);
}

