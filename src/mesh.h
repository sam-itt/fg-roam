#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>
#include <cglm/cglm.h>

#include "btg-io.h"
#include "texture.h"

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

typedef enum{
    PositionBuffer,
    TexCoordBuffer,
    ElementBuffer,
    NBuffers
}VGroupBuffer;

typedef struct{
    /*Texture associated with this mesh*/
    Texture *texture;

    /*Vertex attributes*/
    SGVec3f *positions; /*Vertex coordinates*/
    SGVec2f *texcoords; /*Texture coordinates*/
    indice_t n_vertices; /*Actual vertices in the arrays (vertices, texs)*/
    indice_t allocated_vertices; /*Allocated size*/

    /*Indices into the above data, to be passed to OpenGL in drawElements*/
    indice_t *indices;
    size_t n_indices; /*Actual number of valid indices*/
    size_t allocated_indices; /*We have room to store allocated_indices indices*/

    /*Buffers: OpenGL handles*/
    GLuint buffers[NBuffers];
}VGroup;


typedef struct{
    VGroup *groups;
    size_t n_groups;

    mat4 transformation;
}Mesh;

VGroup *vgroup_init(VGroup *self, const char *material, size_t n_triangles);
void vgroup_dispose(VGroup *self);
long vgroup_add_vertex(VGroup *self, SGVec3d *v, SGVec2f *tex);
bool vgroup_add_triangle(VGroup *self, SGVec3d *v1, SGVec2f *t1, SGVec3d *v2, SGVec2f *t2, SGVec3d *v3, SGVec2f *t3);
size_t vgroup_get_size(VGroup *self, bool data_only);

Mesh *mesh_new_from_file(const char *filename);
Mesh *mesh_new_from_btg(const char *filename);
Mesh *mesh_new(size_t size);
Mesh *mesh_new_empty(void);
void mesh_free(Mesh *self);
bool mesh_set_size(Mesh *self, size_t size);
VGroup *mesh_add_vgroup(Mesh *self, const char *material, size_t n_triangles);

Mesh *mesh_prepare(Mesh *self);
void mesh_render_buffer(Mesh *self, GLuint position, GLuint texcoords, GLuint u_mvp, mat4 vp);

void mesh_dump(Mesh *self);

#endif
