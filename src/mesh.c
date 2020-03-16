#include <stdio.h>
#include <stdlib.h>

#include "mesh.h"
#include "btg-io.h"
#include "texture.h"

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

VGroup *vgroup_init(VGroup *self, size_t size)
{
    self->verts = calloc(size, sizeof(SGVec3d));
    self->texs = calloc(size, sizeof(SGVec2f));
    self->n_vertices = size;

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

Mesh *load_terrain(char *filename)
{
    Mesh *rv;
    SGBinObject *terrain;

    terrain = sg_bin_object_new();
    sg_bin_object_load(terrain,"../test/btg/2990336.btg");


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
            //

            // write group headers
//            printf("\n");
//            printf("# usemtl %s\n", material);
            // write groups
            vgroup_init(&(rv->groups[g_idx]), (end-start)*3);
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
                    rv->groups[g_idx].texs[vidx].x = tex.x;
                    rv->groups[g_idx].texs[vidx].y = tex.y;
                    rv->groups[g_idx].verts[vidx].x = vert.x+terrain->gbs_center.x;
                    rv->groups[g_idx].verts[vidx].y = vert.y+terrain->gbs_center.y;
                    rv->groups[g_idx].verts[vidx].z = vert.z+terrain->gbs_center.z;

                    vidx++;
//                    rv->groups[g_idx].n_vertices = vidx+1;
                }
            }
            g_idx++;
//            rv->n_groups = g_idx+1;

            start = end;
            end = start + 1;
        }
    }
    return rv;
}


void mesh_render(Mesh *self)
{
    VGroup *group;

    glPushMatrix();
    for(int i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        glBindTexture(GL_TEXTURE_2D, group->tex_id);
        glBegin(GL_TRIANGLES);
        for(int j = 0; j < group->n_vertices; j++){
            glTexCoord2f(group->texs[j].x, group->texs[j].y);
            glVertex3f(group->verts[j].x,
                       group->verts[j].y,
                       group->verts[j].z);
        }
        glEnd();
    }
    glPopMatrix();
}

