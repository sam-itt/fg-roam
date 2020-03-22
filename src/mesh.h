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

    /*Buffers: OpenGL handles*/
    GLuint element_buffer;
    GLuint vertex_buffer;
    GLuint texs_buffer;
}VGroup;


typedef struct{
    GLuint n_groups;
   
    VGroup *groups;
}Mesh;


Mesh *mesh_new_from_file(const char *filename);
Mesh *load_terrain(const char *filename);
void mesh_free(Mesh *self);

size_t mesh_get_size(Mesh *self, bool data_only);
void mesh_render(Mesh *self, SGVec3d *epos, double vis);
void mesh_render_buffer(Mesh *self, GLuint position, GLuint texcoords);

Mesh *mesh_prepare(Mesh *self);
#endif
