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


    /*Compute rotation matrix for rotating Y of 90°*/
    mat3 res;
    mat3 expected_res={
        {0,0,-1},
        {0,1,0},
        {1,0,0}
    };
    geo_mat3_rot((vec3){0,1,0}, glm_rad(90), res);
    for(int i = 0; i < 3; i++){
        if(!glm_vec3_eqv_eps(res[i], expected_res[i])){
            printf("col [%d] mismatch\n",i);
            printf("got:\n");
            vec3_dump(res[i]);
            printf("expected:\n");
            vec3_dump(expected_res[i]);
            ev = EXIT_FAILURE;
        }
    }

    /* Rotating vector (2,0,0) using previously computed rotation matrix
     * Note that res is used in favor of expected_res before we are not testing
     * glm_vec3_rotate_m3 here, but the ability to get from (2,0,0)
     * to (0,0,-2) using the equation and values from the example in paper
     * DSTO-TN-0640
     * i.e if res would fail, there is no point succeding here.
     * */ 
    vec3 rotated;
    vec3 expected_rotated = {0,0,-2};
    glm_vec3_rotate_m3(res, (vec3){2,0,0}, rotated);
    if(!glm_vec3_eqv_eps(rotated, expected_rotated)){
        printf("Rotation failure, got:\n");
        vec3_dump(rotated);
        printf("expected:\n");
        vec3_dump(expected_rotated);
        ev = EXIT_FAILURE;
    }


    const double *a, *b;
    vec3 ac, bc,ba;
    a = llhxyz(-34.9, 138.5, 0);
    ac[0] = a[0]*1000.0;
    ac[1] = a[1]*1000.0;
    ac[2] = a[2]*1000.0;
    b = llhxyz(50.8, 4.3, 0);
    bc[0] = b[0]*1000.0;
    bc[1] = b[1]*1000.0;
    bc[2] = b[2]*1000.0;

    /*ECEF coords in meters*/
    ba[0] = bc[0] - ac[0];
    ba[1] = bc[1] - ac[1];
    ba[2] = bc[2] - ac[2];
    printf("a: X: %f, Y: %f, Z:%f\n",ac[0],ac[1],ac[2]);
    printf("b: X: %f, Y: %f, Z:%f\n",b[0]*1000.0,b[1]*1000.0,b[2]*1000.0);
    printf("ba: X: %f, Y: %f, Z:%f\n",ba[0]*1000.0,ba[1]*1000.0,ba[2]*1000.0);
    vec3 rba = {ba[0],ba[1],ba[2]};

    vec3 n, e, d;
    vec3 expected_n = {-0.429, 0.379, 0.820};
    vec3 expected_e = {-0.663, -0.749, 0.0};
    vec3 expected_d = {0.614, -0.543, 0.572};

    geo_get_ned(-34.9, 138.5, n, e, d);
    for(int i = 0; i < 3; i++){
        if(!same_sign(n[i], expected_n[i]) || trunc(100. * local_round(n[i])) != trunc(100. *local_round(expected_n[i]))){
            printf("n mismatch. got: ");
            vec3_dump(n);
            printf("expected:\n");
            vec3_dump(expected_n);
            ev = EXIT_FAILURE;
        }
        if(!same_sign(e[i], expected_e[i]) || trunc(100. * local_round(e[i])) != trunc(100. * local_round(expected_e[i]))){
            printf("e mismatch. got: ");
            vec3_dump(e);
            printf("expected:\n");
            vec3_dump(expected_e);
            ev = EXIT_FAILURE;
        }
        if(!same_sign(d[i], expected_d[i]) || trunc(100. * local_round(d[i])) != trunc(100. * local_round(expected_d[i]))){
            printf("e mismatch. got: ");
            vec3_dump(d);
            printf("expected:\n");
            vec3_dump(expected_d);
            ev = EXIT_FAILURE;
        }

    }
    vec3_dump(n);
    vec3_dump(e);
    vec3_dump(d);
    double north = glm_vec3_dot(rba,n);
    double east = glm_vec3_dot(rba,e);
    printf("north: %0.2f, east: %0.2f\n",north,east);

    const double *values = geo_llh_to_dis(-34.9, 138.5, 10000, 135, 20, 30);
    double expected_values[] = {-3928.261,3475.431,-3634.495,-122.970,47.786,-29.670};
    for(int i = 0; i < 6; i++){
        if(!same_sign(values[i], expected_values[i]) || trunc(100. * local_round(values[i])) != trunc(100. *local_round(expected_values[i]))){
            printf("values mismatch. Got %.2f, expected %.2f\n",values[i],expected_values[i]);
            ev = EXIT_FAILURE;
        }
    }
//    printf()
//    geo_llh_to_dis(44.45195, 5.72632, 852, 0, 0, 0);
	exit(ev);
}

