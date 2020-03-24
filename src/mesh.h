#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>
#include "btg-io.h"

#define USE_INT_INDICES 0

/* This will define the max indice value that can
 * be stored, not the max number of indices
 * OpenGL ES 2.0 wants USHORT indices and we
 * want support it
 * */
#if USE_INT_INDICES
typedef GLuint indice_t;
#define INDICE_TYPE GL_UNSIGNED_INT
#define INDICE_MAX UINT32_MAX
#else
typedef GLushort indice_t;
#define INDICE_TYPE GL_UNSIGNED_SHORT
#define INDICE_MAX UINT16_MAX
#endif



typedef struct{
    GLuint tex_id;

    GLuint n_vertices;

    SGVec3d *verts;
    size_t nverts; /*Number of elements in the vector*/
    size_t vert_esize; /*We have room for esize elements (i.e size NOT in bytes)*/

    SGVec2f *texs;

    indice_t *indices;
    size_t n_indices;

    /*Buffers: OpenGL handles*/
    GLuint element_buffer;
    GLuint vertex_buffer;
    GLuint texs_buffer;

    GHashTable *global_lookup;
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

void mesh_dump_buffer(Mesh *self);

#endif
