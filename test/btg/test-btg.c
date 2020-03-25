#include <stdio.h>
#include <stdlib.h>

#include "btg-io.h" 

int main(int argc, char *argv[])
{
    SGBinObject *obj;

    obj = sg_bin_object_new();

    sg_bin_object_load(obj,"2990336.btg");
//    printf("%s version: %d\n", "3039642.btg.gz", obj->version);

    sg_bin_object_write_obj(obj, "test.obj");
    sg_bin_object_free(obj);

    obj = sg_bin_object_new();
    sg_bin_object_load(obj,"3039642.btg.gz");
    sg_bin_object_write_obj(obj, "test-v7.obj");
    sg_bin_object_free(obj);
//    printf("%s version: %d\n", "3039642.btg.gz", obj->version);
	exit(EXIT_SUCCESS);
}

