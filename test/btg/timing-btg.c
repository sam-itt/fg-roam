#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "btg-io.h"

int main(int argc, char *argv[])
{
    SGBinObject *obj;
    clock_t start, end;

    if(argc < 3){
        printf("Usage: %s filaname.btg[.gz] output\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    start = clock();

    obj = sg_bin_object_new();

    sg_bin_object_load(obj, argv[1]);
//    printf("%s version: %d\n", "3039642.btg.gz", obj->version);

    sg_bin_object_write_obj(obj, argv[2]);
    end = clock();
    fprintf(stderr,"Total duration(w/o freeing): %f\n",(double)(end - start) / CLOCKS_PER_SEC);
    sg_bin_object_free(obj);

//    printf("%s version: %d\n", "3039642.btg.gz", obj->version);
	exit(EXIT_SUCCESS);
}

