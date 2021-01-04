#ifndef MESH_H
#define MESH_H

#include <GL/glut.h>
#include <cglm/cglm.h>

#include "texture.h"
#include "basic-shader.h"
#include "vertex-set.h"
#include "indice.h"
#include "sg-sphere.h"

typedef enum{
    PositionBuffer,
    TexCoordBuffer,
    ElementBuffer,
    NBuffers
}VGroupBuffer;

typedef struct{
    bool prepared;

    char *material;
    /*Texture associated with this mesh*/
    Texture *texture;

    /*Vertices, in a specialized hash*/
    VertexSet *vset;

    /*Vertex attributes, flattened from above hash*/
    SGVec3f *positions; /*Vertex coordinates*/
    SGVec2f *texcoords; /*Texture coordinates*/
    indice_t n_vertices; /*Actual vertices in the arrays (vertices, texs)*/

    /*Indices into the above data, to be passed to OpenGL in drawElements*/
    indice_t *indices;
    size_t n_indices; /*Actual number of valid indices*/
    size_t allocated_indices; /*We have room to store allocated_indices indices*/

    /*In world coordinates, i.e already transformed
     * during prepare stage*/
    SGSphered bs; /*bounding sphere*/

    /*Buffers: OpenGL handles*/
    GLuint buffers[NBuffers];
}VGroup;


typedef struct _Mesh{
    VGroup *groups;
    size_t n_groups;

    mat4d transformation;

    /*In world coordinates, i.e already transformed
     * during prepare stage*/
    SGSphered bs;
}Mesh;

VGroup *vgroup_init(VGroup *self, const char *material, size_t n_triangles);
void vgroup_dispose(VGroup *self);
long vgroup_add_vertex(VGroup *self, SGVec3d *v, SGVec2f *tex);
bool vgroup_add_triangle(VGroup *self, SGVec3d *v1, SGVec2f *t1, SGVec3d *v2, SGVec2f *t2, SGVec3d *v3, SGVec2f *t3);
bool vgroup_finish(VGroup *self, SGSphered *gbs);
size_t vgroup_get_size(VGroup *self, bool data_only);
bool vgroup_prepare(VGroup *self);

Mesh *mesh_new_from_file(const char *filename);
Mesh *mesh_new_from_btg(const char *filename);
Mesh *mesh_new(size_t size);
Mesh *mesh_new_empty(void);
void mesh_free(Mesh *self);
bool mesh_set_size(Mesh *self, size_t size);
VGroup *mesh_add_vgroup(Mesh *self, const char *material, size_t n_triangles);

Mesh *mesh_prepare(Mesh *self);
void mesh_render_buffer(Mesh *self, BasicShader *shader, mat4d vp, vec4 frustum[6], vec4 frustrum_bs);

void mesh_dump(Mesh *self);

#endif
