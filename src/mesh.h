#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>
#include "btg-io.h"


typedef struct{
    GLuint tex_id;
    SGSphered sbound; /*Bounding sphere*/

    GLuint n_vertices;

    SGVec3d *verts;
    size_t nverts; /*Number of elements in the vector*/
    size_t vert_esize; /*We have room for esize elements (i.e size NOT in bytes)*/

    SGVec2f *texs;

    GLuint *indices;
    size_t n_indices;

    /*Buffers: OpenGL handles*/
    GLuint element_buffer;
    GLuint vertex_buffer;
    GLuint texs_buffer;

    GHashTable *global_lookup;
}VGroup;


typedef struct{
    GLuint n_vertices;
    GLuint n_groups;

    SGVec3d *verts;
    size_t nverts; /*Number of elements in the vector*/
    size_t vert_esize; /*We have room for esize elements (i.e size NOT in bytes)*/

    SGVec2f *texs;

    VGroup *groups;

    GLuint vertex_buffer;
    GLuint texs_buffer;

    SGBinObject *terrain;

    GHashTable *stats;
    GHashTable *global_lookup;
    size_t higher_indice_seen;
}Mesh;


Mesh *mesh_new_from_file(const char *filename);
Mesh *load_terrain(const char *filename);
void mesh_free(Mesh *self);

size_t mesh_get_size(Mesh *self, bool data_only);
void mesh_render(Mesh *self, SGVec3d *epos, double vis);
void mesh_render_buffer(Mesh *self, GLuint position, GLuint texcoords);

Mesh *mesh_prepare(Mesh *self);

void mesh_dump_buffer(Mesh *self);

#endif
