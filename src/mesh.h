#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>
#include "btg-io.h"


typedef struct{
    GLuint tex_id;

    GLuint n_vertices;

    SGVec3d *verts;
    SGVec2f *texs;
}VGroup;


typedef struct{
    GLuint n_groups;
   
    VGroup *groups;
}Mesh;


Mesh *load_terrain(char *filename);
void mesh_render(Mesh *self);

#endif
