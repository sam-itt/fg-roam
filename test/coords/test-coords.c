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
    for(int i = 0; i < 3; i++){
        if(rv[i] != expected[i]){
           // printf("%s failure at %d, %a != %a\n","xyzllh",i,rv[i],expected[i]);
            ev = EXIT_FAILURE;
        }
    }

//    printf("rv is : lat:%f lng:%f height:%f\n", rv[0], rv[1], rv[2]);
	exit(ev);
}

