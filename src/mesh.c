#define _GNU_SOURCE
#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#if USE_GLES
#include <SDL2/SDL_opengles2.h>
#include <SDL_opengles2_gl2ext.h>
#else
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#endif

#include "mesh.h"
#include "btg-io.h"
#include "texture.h"
#include "misc.h"

#include "geodesy.h"
#include "cglm/mat4d.h"
#include "frustum-ext.h"
#include "sg-sphere.h"

#include "stg-object.h"

/**
 * @brief Inits a VGroup to make it able to hold as much as @p n_triangles
 * triangles. Will fail (return NULL) if the group as already been inited
 *
 * @param self The VGroup to work on
 * @param material The material name that will serve to lookup the texture
 * @param n_triangles The VGroup will have enough storage for n_triangles
 * triangles
 * @return self on success, NULL on failure.
 */
VGroup *vgroup_init(VGroup *self, const char *material, size_t n_triangles)
{
    /*Non-inited vgroups are memset'ed to 0 by the parent Mesh*/
    if(self->indices)
        return NULL;

    self->material = strdup(material);
//    self->texture = texture_get_by_name(material);

    /*Triangles are described by a set of 3 indices each*/
    self->allocated_indices = n_triangles * 3;
    self->indices = calloc(self->allocated_indices, sizeof(indice_t));

    /* We have the number of indices, but we don't know yet how many different
     * vertices (unique set of positions/texcoords/normals/etc) these indices will
     * index into. Start off with 30% less vertices than indices (optimistic) and let
     * the VertexSet handle it.
     */
    self->vset = vertex_set_new(self->allocated_indices * 0.7);
    if(!self->vset)
        return NULL;
    self->bs.radius = -1.0;
    return self;
}

/**
 * @brief Release memory allocated by the VGgroup.
 *
 * You should not call this function directly, the
 * parent Mesh will take care of the lifecycle of
 * its vgroups.
 *
 * @param self The VGroup worked on
 */
void vgroup_dispose(VGroup *self)
{
    if(self->vset)
        vertex_set_free(self->vset);
    if(self->indices)
        free(self->indices);
    if(self->material)
        free(self->material);
    /*TODO: Interwine the coordinates and propertires to deal with all of the
     * at once*/
    if(self->positions)
        free(self->positions);
    if(self->texcoords)
        free(self->texcoords);
    glDeleteBuffers(NBuffers, self->buffers);
    /*TODO: Release texture*/
}

/**
 * @brief Add a vertex (set of coordinates, texture coordinates, etc.)
 * to the VGgroup at hand and returns its index within the vgroup. That is
 * probably not the function you are looking for.
 *
 * This function will make sure that said vertex appears only once and
 * thus will return a pre-existing index if set passed-in data is already
 * known.
 *
 * @param self The VGroup to work on
 * @param v The vertex coordinates
 * @param tex The texture coordinates associated with the vertex
 * @return The index of the vertex with said properties, or -1 or failure
 * (can't allocate more memory when needed)
 *
 * @see vgroup_add_triangle - This is the function you might be looking for
 */
long vgroup_add_vertex(VGroup *self, SGVec3d *v, SGVec2f *tex)
{
    long rv = 0;
    IndexedVertex *iv;

    iv = vertex_set_add_vertex(self->vset, v, tex);
    if(iv){
        sg_sphered_expand_by(&self->bs, v);
        return iv->index;
    }
    return -1;
}

/**
 * @brief Add a triangle to the VGroup
 *
 * @param self The VGroup to work on
 * @param v1 Position of the first vertex of the triangle
 * @param v2 Position of the second vertex of the triangle
 * @param v3 Position of the thrid vertex of the triangle
 * @param t1 Texture coordinates of the first vertex of the triangle
 * @param t2 Texture coordinates of the second vertex of the triangle
 * @param t3 Texture coordinates of the third vertex of the triangle
 * @return true on success, false on failure
 */
bool vgroup_add_triangle(VGroup *self, SGVec3d *v1, SGVec2f *t1, SGVec3d *v2, SGVec2f *t2, SGVec3d *v3, SGVec2f *t3)
{
    size_t idx[3];

    /*TODO: Handle failure*/
    idx[0] = vgroup_add_vertex(self, v1, t1);
    idx[1] = vgroup_add_vertex(self, v2, t2);
    idx[2] = vgroup_add_vertex(self, v3, t3);

    /* TODO: Auto-split the current group into another group if we reach the maximum number
     * of indices allowed by the storage type (USHORT is the most likely to have the problem)*/
    for(int i = 0; i < 3; i++){
        if(idx[i] < 0)
            return false;
        if(idx[i] > INDICE_MAX){
            printf(
                "WARNING: Terrain %s Group %p has indice value %d greather than "
                "what can be stored with current sizeof(indice_t)(%d), Undefined behavior from now\n",
                "CURRENT FILE", self, idx[i], sizeof(indice_t)
            );
        }
    }

    self->indices[self->n_indices] = idx[0];
    self->indices[self->n_indices+1] = idx[1];
    self->indices[self->n_indices+2] = idx[2];
    self->n_indices += 3;

    return true;
}

/**
 * @brief Computes the memory used by a VGroup
 *
 * @param self The VGroup to work on
 * @param data_only if true, only the actual vertex data is
 * accounted for, discarding counters, handles, and extra
 * memory allocated but not used to store vertices.
 * @return VGroup size in bytes
 */
size_t vgroup_get_size(VGroup *self, bool data_only)
{
    size_t rv = 0;

    if(data_only){
        rv += sizeof(SGVec3f) * self->n_vertices;
        rv += sizeof(SGVec2f) * self->n_vertices;
        rv += sizeof(indice_t) * self->n_indices;
    }else{
        rv += sizeof(SGVec3f) * self->n_vertices;
        rv += sizeof(SGVec2f) * self->n_vertices;
        rv += sizeof(indice_t) * self->allocated_indices;
        rv += sizeof(VGroup);
    }
    return rv;
}

bool vgroup_finish(VGroup *self, SGSphered *gbs)
{
    if(!self->positions){
        bool rv;
        rv = vertex_set_flatten(self->vset, &self->n_vertices, &self->positions, &self->texcoords);
        if(!rv)
            printf("Flattening failed, problems ahead !!\n");
        self->vset = vertex_set_free(self->vset);
        self->bs.center.x += gbs->center.x;
        self->bs.center.y += gbs->center.y;
        self->bs.center.z += gbs->center.z;
#if 0
        printf("VGroup %p bs: %.4f %.4f %.4f %.2f\n",self,
            self->bs.center.x,
            self->bs.center.y,
            self->bs.center.z,
            self->bs.radius
        );
#endif
        return rv;
    }
    return false;
}

/**
 * @brief Allocate various OpenGL resources for rendering. Just need to be
 * called once.
 *
 * @param self The VGroup to work on.
 * @return true when the VGroup has been successfully prepared.
 */
bool vgroup_prepare(VGroup *self)
{
    if(self->prepared) return true;

    self->texture = texture_get_by_name(self->material);

    glGenBuffers(NBuffers, self->buffers);

    glBindBuffer(GL_ARRAY_BUFFER, self->buffers[PositionBuffer]);
    glBufferData(
        GL_ARRAY_BUFFER,
        self->n_vertices*sizeof(SGVec3f),
        self->positions,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ARRAY_BUFFER, self->buffers[TexCoordBuffer]);
    glBufferData(
        GL_ARRAY_BUFFER,
        self->n_vertices*sizeof(SGVec2f),
        self->texcoords,
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->buffers[ElementBuffer]);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        self->n_indices*sizeof(indice_t),
        self->indices,
        GL_STATIC_DRAW
    );

    self->prepared = true;
    return true;
}



/**
 * @brief Creates a new mesh with @p size groups.
 *
 * @param size Number of vgroups.
 * @return Newly-created mesh on success, NULL otherwise.
 *
 * @see mesh_new_empty
 * @see mesh_set_size
 */
Mesh *mesh_new(size_t size)
{
    Mesh *rv;

    rv = mesh_new_empty();
    if(!mesh_set_size(rv, size)){
        mesh_free(rv);
        return NULL;
    }
    return rv;
}

/**
 * @brief Creates a new mesh with no storage for groups.
 *
 * Size must be set before starting to access groups
 *
 * @return Newly-created group on success, NULL otherwise.
 *
 * @see mesh_set_size
 */
Mesh *mesh_new_empty(void)
{
    Mesh *rv;
    rv = calloc(1, sizeof(Mesh));
    if(rv){
        glm_mat4d_identity(rv->transformation);
    }
    return rv;
}

void mesh_add_accessory(Mesh *self, Mesh *accessory)
{
    Mesh *iter;

    for(iter = self; iter->next != NULL; iter = iter->next);

    iter->next = accessory;
}

/**
 * Creates and prepares a new mesh from a STG file
 *
 * @param filename The filename to read from
 * @return a newly created and prepared Mesh
 */
Mesh *mesh_new_from_file(const char *filename)
{
    Mesh *rv;
    StgObject stg;
    char *obj_fname;
    size_t n;
    bool found;

    if(!stg_object_init(&stg, filename))
        return NULL;

    obj_fname = NULL;
    fseek(stg.fp,  0L, SEEK_SET);
    found = stg_object_get_value(&stg, "OBJECT_BASE", true, &obj_fname, &n);
    if(!found) //stg file has no base object, can't do nothing
        goto bail;

    rv = mesh_new_from_btg(obj_fname);
    if(!rv)
        goto bail;
    for(size_t i = 0; i < rv->n_groups; i++){
        vgroup_finish(&(rv->groups[i]), &rv->bs);
    }

    /*Load tile accessories e.g airports*/
    fseek(stg.fp,  0L, SEEK_SET);
    while((found = stg_object_get_value(&stg, "OBJECT", true, &obj_fname, &n))){
        Mesh *acc = mesh_new_from_btg(obj_fname);
        if(!acc){
            printf("%s loading failed, skipping\n",obj_fname);
            continue;
        }
        for(size_t i = 0; i < acc->n_groups; i++){
            vgroup_finish(&(acc->groups[i]), &acc->bs);
        }
        mesh_add_accessory(rv, acc);
    }

bail:
    if(obj_fname)
        free(obj_fname);
    stg_object_dispose(&stg);
    return rv;
}


/**
 * @brief Release memory hold by the mesh
 *
 * @param self The mesh to free
 */
void mesh_free(Mesh *self)
{
    Mesh *iter, *next;
    VGroup *group;

    iter = self;
    while(iter){
        next = iter->next;

        for(size_t i = 0; i < iter->n_groups; i++){
            group = &(iter->groups[i]);
            vgroup_dispose(group);
        }
        if(iter->groups)
            free(iter->groups);
        free(iter);

        iter = next;
    }
}

bool mesh_set_size(Mesh *self, size_t size)
{
    VGroup *groups;
    size_t old_size;

    old_size = self->n_groups;
    groups = realloc(self->groups, sizeof(VGroup) * size);
    if(groups){
        self->groups = groups;
        memset(self->groups+(old_size*sizeof(VGroup)), 0, (size-old_size)*sizeof(VGroup));
        self->n_groups = size;
    }
    return groups != NULL;
}

/**
 * @brief Adds a new vgroup in @p self that can hold up to @p n_triangles
 * triangles.
 *
 * There can be more than one group with the same material within the same
 * mesh, therefore this function will always use the first available vgroup
 * slot in the mesh.
 *
 * @param self The mesh to work on.
 * @param material Material name (usually matches texture and other properties)
 * @param n_triangles The number of triangles that make up this group
 * @return The VGroup or NULL on failure
 */
VGroup *mesh_add_vgroup(Mesh *self, const char *material, size_t n_triangles)
{
    for(int i = 0; i < self->n_groups; i++){
        /*First available group will have all it's pointers set to NULL*/
        if(!self->groups[i].indices){
            return vgroup_init(&(self->groups[i]), material, n_triangles);
        }
    }
    return NULL;
}

/**
 * @brief Allocate various OpenGL resources for rendering. Just need to be
 * called once.
 *
 * @param self The mesh to work on.
 */
Mesh *mesh_prepare(Mesh *self)
{
    for(size_t i = 0; i < self->n_groups; i++){
        vgroup_prepare(&(self->groups[i]));
    }
    return self;
}

/**
 * @brief Does the actual rendering of a prepared mesh.
 *
 * @param Mesh The Mesh to be worked on
 * @param position Handle to "position" attribute of the shader
 * @param texcoords Handle to "texcoords" attribute of the shader
 * @param u_mvp Handle to the Model-View-Projection uniform matrix on the shader
 * @param vp The current View-Projection matrix.
 */
void mesh_render_buffer(Mesh *self, BasicShader *shader, mat4d vp, vec4 frustum[6], vec4 frustrum_bs)
{
    /* TODO: Maybe get the mv out of here (rendermaanger that applies the matrix
     * before calling mesh_render_buffer?).
     * */

    VGroup *group;
    mat4d mvp;
    mat4 mvpf;
    vec4 mbs = {self->bs.center.x,self->bs.center.y,self->bs.center.z,self->bs.radius};

    if(!glm_sphere_sphere(frustrum_bs, mbs)){/*printf("sphere-culled mesh %p\n",self);*/ return;}
    if(!glm_frustum_cgsphered(frustum, &self->bs)) return;

    glm_mat4d_mul(vp, self->transformation, mvp);
    glm_mat4d_ucopyf(mvp, mvpf);
    glUniformMatrix4fv(shader->mvp, 1, GL_FALSE, mvpf[0]);

    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        vec4 gbs = {group->bs.center.x,group->bs.center.y,group->bs.center.z,group->bs.radius};

        if(!glm_sphere_sphere(frustrum_bs, gbs)){continue;}
        if(!glm_frustum_cgsphered(frustum, &group->bs)) {continue;}

        /*TODO: static_branch on preparation*/
        if(!group->prepared)
            vgroup_prepare(group);

        glActiveTexture(GL_TEXTURE0 );
        glBindTexture(GL_TEXTURE_2D, group->texture ? group->texture->id : 0); /*TODO: static_branch on tex loading*/

        glEnableVertexAttribArray(shader->position);
        glBindBuffer(GL_ARRAY_BUFFER, group->buffers[PositionBuffer]);
        glVertexAttribPointer(
            shader->position,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(SGVec3f), /*If we don't specify the stride, apitrace doesn't detect the values correctly*/
            (void*)0
        );

        glEnableVertexAttribArray(shader->texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, group->buffers[TexCoordBuffer]);
        glVertexAttribPointer(
            shader->texcoords,
            2,
            GL_FLOAT,
            GL_FALSE,
            sizeof(SGVec2f), /*As above*/
            (void*)0
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group->buffers[ElementBuffer]);
        glDrawElements(GL_TRIANGLES, group->n_indices, INDICE_TYPE, 0);

        glDisableVertexAttribArray(shader->position);
        glDisableVertexAttribArray(shader->texcoords);
    }
}

Mesh *mesh_new_from_btg(const char *filename)
{
    Mesh *rv = NULL;
    SGBinObject *terrain;

    terrain = sg_bin_object_new();
    sg_bin_object_load(terrain, filename);

    if(terrain->tris_v->len == 0)
        goto out;

    /*Do a first pass to read the total number of triangle groups*/
    guint start = 0;
    guint end = 1;
    char *material;
    size_t ngroups = 0;

    while ( start < terrain->tri_materials->len ) {
        // find next group
        material = g_ptr_array_index(terrain->tri_materials,start);
        while ( (end < terrain->tri_materials->len) &&
                (!strcmp(material, g_ptr_array_index(terrain->tri_materials,end))) )
        {
            end++;
        }
        ngroups++;
        start = end;
        end = start + 1;
    }

    rv = mesh_new(ngroups);
    glm_translated(rv->transformation,
        (vec3d){terrain->gbs_center.x,
                terrain->gbs_center.y,
                terrain->gbs_center.z
        }
    );
    rv->bs = (SGSphered){
        .center = terrain->gbs_center,
        .radius = terrain->gbs_radius
    };

    /*Second pass, actually read the data*/
    start = 0;
    end = 1;
    VGroup *group;
    while ( start < terrain->tri_materials->len ) {
        // find next group
        material = g_ptr_array_index(terrain->tri_materials,start);
        while ( (end < terrain->tri_materials->len) &&
                (!strcmp(material, g_ptr_array_index(terrain->tri_materials,end))) )
        {
            end++;
        }
        /*Current group spans from tris_v[start] to tris_v[end-1]*/

        group = mesh_add_vgroup(rv, material, (end-start)*3);
        if(!group){
            printf("Couldn't get group for %s size %d\n",material,  (end-start)*3);
            exit(-1);

        }
        for (guint  i = start; i < end; ++i ) {
            GArray *tri_v = g_ptr_array_index(terrain->tris_v, i);
            GArray *tri_c = g_ptr_array_index(terrain->tris_c, i);
            GPtrArray *tri_tcs = g_ptr_array_index(terrain->tris_tcs,i);
            GArray *ttcs = g_ptr_array_index(tri_tcs,0);

            for (guint j = 2; j < tri_v->len; j += 3) { //Edges of the triangle
                /* Here we have take the same approach as Simgear that is
                 * starting on the last edge(vertex) of the triangle tri_v[2]
                 * and going backwards(tri_v[2-1], tri_v[2-2]), reading one
                 * triangle per operation while eliminating tests on i-1,i-2.
                 * SimGear seems to consider that more than one triangle could
                 * be in tri_v and goes by increments of 3. That has been
                 * reproduced here.
                 * */
                int a3, b3;
                if(terrain->version >= 10){
                    a3 = g_array_index(tri_v, int, j);
                    b3 = g_array_index(ttcs, int, j);
                }else{
                    a3 = g_array_index(tri_v, uint16_t, j);
                    b3 = g_array_index(ttcs, uint16_t, j);
                }
                SGVec3d vert3 = g_array_index(terrain->wgs84_nodes, SGVec3d, a3);
                SGVec2f tex3 = g_array_index(terrain->texcoords, SGVec2f, b3);

                int a2, b2;
                if(terrain->version >= 10){
                    a2 = g_array_index(tri_v, int, j - 1);
                    b2 = g_array_index(ttcs, int, j - 1);
                }else{
                    a2 = g_array_index(tri_v, uint16_t, j - 1);
                    b2 = g_array_index(ttcs, uint16_t, j - 1);
                }
                SGVec3d vert2 = g_array_index(terrain->wgs84_nodes, SGVec3d, a2);
                SGVec2f tex2 = g_array_index(terrain->texcoords, SGVec2f, b2);

                int a1, b1;
                if(terrain->version >= 10){
                    a1 = g_array_index(tri_v, int, j - 2);
                    b1 = g_array_index(ttcs, int, j - 2);
                }else{
                    a1 = g_array_index(tri_v, uint16_t, j - 2);
                    b1 = g_array_index(ttcs, uint16_t, j - 2);
                }
                SGVec3d vert1 = g_array_index(terrain->wgs84_nodes, SGVec3d, a1);
                SGVec2f tex1 = g_array_index(terrain->texcoords, SGVec2f, b1);

                vgroup_add_triangle(group,
                    &vert1, &tex1,
                    &vert2, &tex2,
                    &vert3, &tex3
                );
            }
        }
        start = end;
        end = start + 1;
    }
out:
    sg_bin_object_free(terrain);
    return rv;
}

/**
 * @brief Computes the memory used by a Mesh
 *
 * @param self The Mesh to work on
 * @param data_only if true, only the actual vertex data is
 * accounted for, discarding counters, handles, and extra
 * memory allocated but not used to store vertices.
 * @return Mesh size in bytes
 */
size_t mesh_get_size(Mesh *self, bool data_only)
{
    size_t rv;

    rv = 0;
    for(size_t i = 0; i < self->n_groups; i++){
        rv += vgroup_get_size(&(self->groups[i]), data_only);
    }

    if(!data_only)
        rv += sizeof(Mesh);
    return rv;
}

/**
 * @brief Show the Mesh vertices data for debugging purposes
 *
 *
 */
void mesh_dump(Mesh *self)
{
    VGroup *group;
    printf("Dumping Mesh %p\n",self);
    for(size_t i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        printf("Group #%d (%s) %d indices:\n",i,
            group->texture->name,
            group->n_indices
        );
        if(group->n_indices%3 != 0)
            printf("WARNING: Group #%d as a number of vertices that don't match a set of triangles\n",i);
        for(int j = 0; j < group->n_indices; j++){
            printf("indice[%d] -> Vertex[%d]: pos:%0.5f %0.5f %0.5f tex: %0.5f %0.5f\n",
                   j, group->indices[j],
                   group->positions[group->indices[j]].x,
                   group->positions[group->indices[j]].y,
                   group->positions[group->indices[j]].z,
                   group->texcoords[group->indices[j]].x,
                   group->texcoords[group->indices[j]].y
            );
        }
    }
    printf("Mesh %p dumped\n", self);
}
