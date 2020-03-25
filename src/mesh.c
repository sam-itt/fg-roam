#define _GNU_SOURCE
#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "mesh.h"
#include "btg-io.h"
#include "texture.h"
#include "misc.h"


#if 0
VGroup *vgroup_new(size_t size)
{
    VGroup *rv;

    rv = calloc(1, sizeof(VGroup));
    if(rv){
        rv->verts = calloc(size, sizeof(SGVec3d));
        rv->texs = calloc(size, sizeof(SGVec2f));
    }
    return rv;
}
#endif

VGroup *vgroup_init(VGroup *self, size_t size, bool self_clear)
{
    if(self_clear)
        memset(self, 0, sizeof(VGroup));

    self->vert_esize = size;
    self->verts = calloc(self->vert_esize, sizeof(SGVec3d));
    self->texs = calloc(size, sizeof(SGVec2f));
    self->indices = calloc(size, sizeof(indice_t));
//    printf("allocated %d indices at %p\n",size,self->indices);
    self->n_vertices = size;

    self->global_lookup = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);

    return self;
}

void vgroup_dispose(VGroup *self)
{
    free(self->verts);
    free(self->texs);
    free(self->indices);
    g_hash_table_unref(self->global_lookup);
}

size_t vgroup_get_size(VGroup *self, bool data_only)
{
    size_t rv = 0;

    rv += sizeof(SGVec3d) * self->vert_esize;
    rv += sizeof(SGVec2f) * self->vert_esize;
    rv += sizeof(indice_t) * self->n_vertices;
    if(!data_only)
        rv += sizeof(VGroup);
    return rv;
}

Mesh *mesh_new(size_t size)
{
    Mesh *rv;
    rv = calloc(1, sizeof(Mesh));
    if(rv){
        rv->groups = calloc(size, sizeof(VGroup));
        rv->n_groups = size;

    }
    return rv;
}

Mesh *mesh_new_from_file(const char *filename)
{
    Mesh *rv;
   rv = load_terrain(filename);

    mesh_prepare(rv);
    return rv;
}


void mesh_free(Mesh *self)
{
    VGroup *group;

    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        vgroup_dispose(group);
    }
    free(self->groups);
    free(self);
}

size_t mesh_get_size(Mesh *self, bool data_only)
{
    VGroup *group;
    size_t rv;

    rv = 0;

    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        rv += vgroup_get_size(group, data_only);
    }

    if(!data_only)
        rv += sizeof(Mesh);
    return rv;
}


Mesh *mesh_prepare(Mesh *self)
{

    VGroup *group;

    for(unsigned int i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);

        glGenBuffers(1, &(group->vertex_buffer));
        glBindBuffer(GL_ARRAY_BUFFER, group->vertex_buffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            group->nverts*sizeof(SGVec3d),
            group->verts,
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &group->texs_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, group->texs_buffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            group->n_vertices*sizeof(SGVec2f),
            group->texs,
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &(group->element_buffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group->element_buffer);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, 
            group->n_vertices*sizeof(indice_t),
            group->indices, 
            GL_STATIC_DRAW
        );
    }
    printf("Mesh %p prepared\n", self);
//    mesh_dump_buffer(self);
    size_t sze = mesh_get_size(self, false);
    printf("Size: %f %s\n", get_sized_unit_value(sze), get_sized_unit_text(sze));

//    mesh_show_vertex_use(self);
//    exit(0);
    return self;
}

void mesh_dump_buffer(Mesh *self)
{
    VGroup *group;
    printf("Dumping Mesh\n");
    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        printf("Group %d/%d %d vertices\n",i ,self->n_groups-1, group->n_vertices);
        for(GLuint j = 0; j < group->n_vertices; j++){
            printf("#%d: %0.5f %0.5f %0.5f\n",
                   j, group->verts[group->indices[j]].x, 
                   group->verts[group->indices[j]].y, 
                   group->verts[group->indices[j]].z);
        }

    }
    printf("Mesh dumped\n");

}

size_t vgroup_add_vertex(VGroup *self, SGVec3d *v, SGVec2f *tex, int global_idx)
{
    size_t rv = 0;
    gpointer existing;

    existing = g_hash_table_lookup(self->global_lookup, GINT_TO_POINTER(global_idx));
    if(existing){
        rv = GPOINTER_TO_UINT(existing);
        if(self->texs[rv].x == tex->x && self->texs[rv].y == tex->y)
            return rv;
    }

    if(self->nverts == self->vert_esize){
        self->vert_esize += 16;
        self->verts = reallocarray(self->verts,  self->vert_esize, sizeof(SGVec3d));
        if(!self->verts){
            printf("Realloc failure\n");
            exit(-1);
        }
    }
    /*If we get here, we need to add v into the array*/
    rv = self->nverts;
    self->verts[rv] = (SGVec3d){v->x,v->y,v->z};
    self->texs[rv].x = tex->x;
    self->texs[rv].y = tex->y;
    self->nverts++;

    g_hash_table_insert(self->global_lookup, GINT_TO_POINTER(global_idx), GUINT_TO_POINTER(rv));

    return(rv);
}



Mesh *load_terrain(const char *filename)
{
    Mesh *rv = NULL;
    SGBinObject *terrain;

    terrain = sg_bin_object_new();
    sg_bin_object_load(terrain, filename);


    if ( terrain->tris_v->len != 0 ) {
        //printf("# triangle groups\n");

        guint start = 0;
        guint end = 1;
        char *material;
        size_t ngroups = 0;
        while ( start < terrain->tri_materials->len ) {
            // find next group
            material = g_ptr_array_index(terrain->tri_materials,start); 
           // printf("tri_materials.size: %d\n", terrain->tri_materials->len);
            while ( (end < terrain->tri_materials->len) &&
                    (!strcmp(material, g_ptr_array_index(terrain->tri_materials,end))) )
            {
                //printf("end = %d\n",end);
                end++;
            }

            ngroups++;

            start = end;
            end = start + 1;
        }


        rv = mesh_new(ngroups);
        size_t g_idx = 0;
        start = 0;
        end = 1;

        while ( start < terrain->tri_materials->len ) {
            // find next group
            material = g_ptr_array_index(terrain->tri_materials,start); 
           // printf("tri_materials.size: %d\n", terrain->tri_materials->len);
            while ( (end < terrain->tri_materials->len) &&
                    (!strcmp(material, g_ptr_array_index(terrain->tri_materials,end))) )
            {
                //printf("end = %d\n",end);
                end++;
            }
            //printf("group = %d to %d\n",start, end-1);


            // write group headers
//            printf("\n");
//            printf("# usemtl %s\n", material);
            // write groups
            vgroup_init(&(rv->groups[g_idx]), (end-start)*3, false);
            rv->groups[g_idx].tex_id = texture_get_id_by_name(material);
            size_t vidx = 0;
            for (guint  i = start; i < end; ++i ) {
                GArray *tri_v = g_ptr_array_index(terrain->tris_v, i);
                GArray *tri_c = g_ptr_array_index(terrain->tris_c, i);
                GPtrArray *tri_tcs = g_ptr_array_index(terrain->tris_tcs,i);
                if(tri_v->len != 3){
                    printf("Wrong tri_v->len: %d\n",tri_v->len);
                    exit(EXIT_FAILURE);
                }
                if(tri_c->len > 0)
                    printf("Triangle %d has %d colors!\n",i,tri_c->len);
                for (guint  j = 0; j < tri_v->len; ++j ) {
                    int a, b;
                    size_t idx;
                    GArray *ttcs = g_ptr_array_index(tri_tcs,0);
                    if(terrain->version >= 10){
                        a = g_array_index(tri_v, int, j);
                        b = g_array_index(ttcs, int, j);
                    }else{
                        a = g_array_index(tri_v, uint16_t, j);
                        b = g_array_index(ttcs, uint16_t, j);
                    }
                    SGVec3d vert = g_array_index(terrain->wgs84_nodes, SGVec3d, a);
                    SGVec2f tex = g_array_index(terrain->texcoords, SGVec2f, b);
                    vert.x += terrain->gbs_center.x;
                    vert.y += terrain->gbs_center.y;
                    vert.z += terrain->gbs_center.z;

                    idx = vgroup_add_vertex(&(rv->groups[g_idx]), &vert, &tex,a);
                    if(idx > INDICE_MAX){
                        printf(
                            "WARNING: Terrain %s Group %d has indice value %d greather than "
                            "what can be stored with current sizeof(indice_t)(%d), Undefined behavior from now\n",
                            filename, g_idx, idx, sizeof(indice_t));
                    }
                    rv->groups[g_idx].indices[vidx] = idx;

                    vidx++;
                }
            }
//            printf("Group %d, final number of vertices %d vs %d\n", g_idx, rv->groups[g_idx].nverts,rv->groups[g_idx].n_vertices);
            g_idx++;

            start = end;
            end = start + 1;
        }
    }
//    rv->terrain = terrain;
    sg_bin_object_free(terrain);
    printf("Done loading terrain\n");
    return rv;
}

void mesh_render_buffer(Mesh *self, GLuint position, GLuint texcoords)
{
    VGroup *group;

    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        glActiveTexture(GL_TEXTURE0 );
        glBindTexture(GL_TEXTURE_2D, group->tex_id);
 
        glEnableVertexAttribArray(position);
        glBindBuffer(GL_ARRAY_BUFFER, group->vertex_buffer);
        glVertexAttribPointer(
            position,
            3, 
            GL_DOUBLE, 
            GL_FALSE, 
            0, /*no need to specify stride on single attribute vectors*/
            (void*)0
        );

        glEnableVertexAttribArray(texcoords);
        glBindBuffer(GL_ARRAY_BUFFER, group->texs_buffer);
        glVertexAttribPointer(
            texcoords,
            2, 
            GL_FLOAT, 
            GL_FALSE, 
            0, /*As above*/
            (void*)0
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group->element_buffer);
        glDrawElements(GL_TRIANGLES, group->n_vertices, INDICE_TYPE, 0);

        glDisableVertexAttribArray(position);
        glDisableVertexAttribArray(texcoords);
    }
//    printf("Mesh %p rendered\n",self);
}

