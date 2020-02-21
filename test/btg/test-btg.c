#include <stdio.h>
#include <stdlib.h>

#include "btg-io.h" 

int main(int argc, char *argv[])
{
    SGBinObject *obj;

    obj = sg_bin_object_new();

    sg_bin_object_load(obj,"2990336.btg");

    sg_bin_object_write_obj(obj, "test.obj");
	exit(EXIT_SUCCESS);
}

