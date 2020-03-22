#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "mesh.h"
#include "btg-io.h"
#include "texture.h"

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

    self->verts = calloc(size, sizeof(SGVec3d));
    self->texs = calloc(size, sizeof(SGVec2f));
    self->n_vertices = size;
    self->sbound.radius = -1.0;

    return self;
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

    for(int i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        free(group->verts);
        free(group->texs);
    }
    free(self->groups);
    free(self);
}

size_t mesh_get_size(Mesh *self, bool data_only)
{
    VGroup *group;
    size_t rv;

    rv = 0;
    for(int i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        if(!data_only)
            rv += sizeof(VGroup);
        rv += sizeof(SGVec3d) * group->n_vertices;
        rv += sizeof(SGVec2f) * group->n_vertices;
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
            group->n_vertices*sizeof(SGVec3d),
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
    }
    printf("Mesh %p prepared\n", self);
    return self;
}



Mesh *load_terrain(const char *filename)
{
    Mesh *rv = NULL;
    SGBinObject *terrain;

    terrain = sg_bin_object_new();
    sg_bin_object_load(terrain, filename);


    if ( terrain->tris_v->len != 0 ) {
        //printf("# triangle groups\n");

        int start = 0;
        int end = 1;
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
            for (int  i = start; i < end; ++i ) {
                GArray *tri_v = g_ptr_array_index(terrain->tris_v, i);
                GArray *tri_c = g_ptr_array_index(terrain->tris_c, i);
                GPtrArray *tri_tcs = g_ptr_array_index(terrain->tris_tcs,i);
                float tex[3];
                if(tri_v->len != 3){
                    printf("Wrong tri_v->len: %d\n",tri_v->len);
                    exit(EXIT_FAILURE);
                }
                if(tri_c->len > 0)
                    printf("Triangle %d has %d colors!\n",i,tri_c->len);
                for (int  j = 0; j < tri_v->len; ++j ) {
                    int a, b;
                    GArray *ttcs = g_ptr_array_index(tri_tcs,0);
                    a = g_array_index(tri_v, int, j);
                    b = g_array_index(ttcs, int, j);
                    SGVec3d vert = g_array_index(terrain->wgs84_nodes, SGVec3d, a);
                    SGVec2f tex = g_array_index(terrain->texcoords, SGVec2f, b);
                    vert.x += terrain->gbs_center.x;
                    vert.y += terrain->gbs_center.y;
                    vert.z += terrain->gbs_center.z;
                    rv->groups[g_idx].texs[vidx].x = tex.x;
                    rv->groups[g_idx].texs[vidx].y = tex.y;
                    rv->groups[g_idx].verts[vidx].x = vert.x;
                    rv->groups[g_idx].verts[vidx].y = vert.y;
                    rv->groups[g_idx].verts[vidx].z = vert.z;
                    sg_sphered_expand_by(&(rv->groups[g_idx].sbound), &vert);

                    vidx++;
                }
            }
            g_idx++;

            start = end;
            end = start + 1;
        }
    }
    return rv;
}

void mesh_render_buffer(Mesh *self, GLuint position, GLuint texcoords)
{
    VGroup *group;

    glPushMatrix();
    for(int i = 0; i < self->n_groups; i++){
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

        glDrawArrays(GL_TRIANGLES, 0, group->n_vertices); // 12*3 indices starting at 0 -> 12 triangles
        glDisableVertexAttribArray(position);
        glDisableVertexAttribArray(texcoords);
    }
    glPopMatrix();
}



void mesh_render(Mesh *self, SGVec3d *epos, double vis)
{
    VGroup *group;
    double vis2;

    vis2 = vis*vis;
    glPushMatrix();
    for(int i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        glBindTexture(GL_TEXTURE_2D, group->tex_id);
        glBegin(GL_TRIANGLES);
#ifdef ENABLE_CENTRIOD_CULLING
        int tidx = 0;
        for(int j = 0; j < group->n_vertices; j+=3, tidx++){
            for(int k = 0; k < 3; k++){
                int idx = j+k;
                glTexCoord2f(group->texs[idx].x, group->texs[idx].y);
                glVertex3f(group->verts[idx].x,
                           group->verts[idx].y,
                           group->verts[idx].z);
            }
        }
#else
        for(int j = 0; j < group->n_vertices; j++){
            glTexCoord2f(group->texs[j].x, group->texs[j].y);
            glVertex3f(group->verts[j].x,
                       group->verts[j].y,
                       group->verts[j].z);
        }

#endif
        glEnd();
    }
    glPopMatrix();
}

