#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>
#include "btg-io.h"


typedef struct{
    GLuint tex_id;
    SGSphered sbound; /*Bounding sphere*/

    GLuint n_vertices;

    SGVec3d *verts;
    SGVec2f *texs;

    SGVec3d *centroids;
}VGroup;


typedef struct{
    GLuint n_groups;
   
    VGroup *groups;
}Mesh;


Mesh *load_terrain(char *filename);
void mesh_free(Mesh *self);

size_t mesh_get_size(Mesh *self, bool data_only);
void mesh_render(Mesh *self, SGVec3d *epos, double vis);

#endif
