#include "cglm/mat4d.h"
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

#include "geodesy.h"
#include "quat-ext.h"

#define vertex_equal(v1, v2) (((v1)->x == (v2)->x) && ((v1)->y == (v2)->y) && ((v1)->z == (v2)->z))
#define textures_equal(t1, t2) ((t1)->x == (t2)->x) && ((t1)->y == (t2)->y)

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
    self->verts = calloc(self->vert_esize, sizeof(SGVec3f));
    self->texs = calloc(size, sizeof(SGVec2f));
    self->indices = calloc(size, sizeof(indice_t));
//    printf("allocated %d indices at %p\n",size,self->indices);
    self->n_vertices = size;

    return self;
}

void vgroup_grow_by(VGroup *self, size_t size)
{
    self->vert_esize += size;
    self->verts = realloc(self->verts, self->vert_esize*sizeof(SGVec3f));
    self->texs = realloc(self->texs, self->vert_esize * sizeof(SGVec2f));
    self->indices = realloc(self->indices, self->vert_esize * sizeof(indice_t));
//    printf("allocated %d indices at %p\n",size,self->indices);
//    self->n_vertices += size;
}

void vgroup_dispose(VGroup *self)
{
    free(self->verts);
    free(self->texs);
    free(self->indices);
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

static int vgroup_compare(VGroup *g1, VGroup *g2)
{
    return strcmp(g1->tex_name, g2->tex_name);
}

size_t vgroup_add_vertex(VGroup *self, SGVec3d *v, SGVec2f *tex)
{
    size_t rv = 0;

    bool dump = !strcmp(self->tex_name,"pa_threshold");

    if(dump){
        printf("vgroup_add_vertex\n"
               "\tposition: %f %f %f\n"
               "\ttex: %f %f\n",
            v->x,
            v->y,
            v->z,
            tex->x,
            tex->y
        );
    }

    for(int i = 0; i < self->nverts; i++){
        if(vertex_equal(&(self->verts[i]), v) && textures_equal(&(self->texs[i]), tex)){
            if(dump){
                printf("Re-using vertex %d (%f,%f,%f) - (%f,%f)\n",i,
                    v->x, v->y, v->z,
                    tex->x, tex->y
                );
            }
            return i;
        }
    }

    if(self->nverts == self->vert_esize){
        self->vert_esize += 16;
        self->verts = reallocarray(self->verts,  self->vert_esize, sizeof(SGVec3f));
        if(!self->verts){
            printf("Realloc failure\n");
            exit(-1);
        }
    }
    /*If we get here, we need to add v into the array*/
    rv = self->nverts;
    self->verts[rv] = (SGVec3f){v->x,v->y,v->z};
    self->texs[rv].x = tex->x;
    self->texs[rv].y = tex->y;
    self->nverts++;

    if(dump){
        printf("Added as vertex %d: (%f,%f,%f) - (%f,%f)\n",rv,
            self->verts[rv].x, self->verts[rv].y, self->verts[rv].z,
            self->texs[rv].x, self->texs[rv].y
        );
    }
    return(rv);
}


void vgroup_add_triangle(VGroup *self, SGVec3d *v1, SGVec2f *t1, SGVec3d *v2, SGVec2f *t2, SGVec3d *v3, SGVec2f *t3)
{
    size_t idx1, idx2, idx3;

    idx1 = vgroup_add_vertex(self, v1, t1);
    idx2 = vgroup_add_vertex(self, v2, t2);
    idx3 = vgroup_add_vertex(self, v3, t3);

    if(idx1 > INDICE_MAX || idx2 > INDICE_MAX || idx3 > INDICE_MAX){
        printf(
            "WARNING: Terrain %s Group %d has indice value %d greather than "
            "what can be stored with current sizeof(indice_t)(%d), Undefined behavior from now\n",
            "CURRENT FILE", -1, idx1, sizeof(indice_t));
    }


    self->indices[self->n_indices] = idx1;
    self->n_indices++;
    self->indices[self->n_indices] = idx2;
    self->n_indices++;
    self->indices[self->n_indices] = idx3;
    self->n_indices++;
}


Mesh *mesh_new(size_t size)
{
    Mesh *rv;
    rv = calloc(1, sizeof(Mesh));
    if(rv){
        rv->groups = calloc(size, sizeof(VGroup));
        rv->n_groups = size;
        glm_mat4d_identity(rv->transformation);
    }
    return rv;
}

Mesh *mesh_new_from_file(const char *filename)
{
    Mesh *rv;
    rv = load_terrain(filename);
    if(rv)
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

int mesh_get_vgroup_idx(Mesh *self, const char *material, size_t create_size)
{
    int k;
    for(k = 0; k < self->n_groups; k++){
        if(self->groups[k].tex_name == NULL)
            break;
        if(!strcmp(self->groups[k].tex_name, material)){
            printf("Found existing VGroup #%d for material %s\n", k, material);
            vgroup_grow_by(&(self->groups[k]), create_size);
            return k;
        }
    }

    if(k == self->n_groups)
        return -1;

    vgroup_init(&(self->groups[k]), create_size, true);
    self->groups[k].tex_id = texture_get_id_by_name(material);
    self->groups[k].tex_name = strdup(material);
    self->groups[k].n_indices = 0;

    printf("Inited VGroup #%d for material %s\n", k, material);

    return k;
}

Mesh *mesh_prepare(Mesh *self)
{

    VGroup *group;
    for(unsigned int i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);

        glGenBuffers(1, &(group->vertex_buffer));
        glBindBuffer(GL_ARRAY_BUFFER, group->vertex_buffer);
        printf("group->nverts: %d, sizeof(SGVec3f): %d\n",group->nverts, sizeof(SGVec3f));
        glBufferData(
            GL_ARRAY_BUFFER,
            group->nverts*sizeof(SGVec3f),
            group->verts,
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &group->texs_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, group->texs_buffer);
        glBufferData(
            GL_ARRAY_BUFFER,
            group->nverts*sizeof(SGVec2f),
            group->texs,
            GL_STATIC_DRAW
        );

        glGenBuffers(1, &(group->element_buffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group->element_buffer);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            group->n_indices*sizeof(indice_t),
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
        printf("Group #%d (%s):\n",i, group->tex_name);
        printf("%d indices\n", group->n_indices);
        if(group->n_indices%3 != 0)
            printf("WARNING group #%d as a number of vertices that don't match a set of triangles\n",i);
        for(int j = 0; j < group->n_indices; j++){
            printf("#%d: %0.5f %0.5f %0.5f\n",
                   j, group->verts[group->indices[j]].x,
                   group->verts[group->indices[j]].y,
                   group->verts[group->indices[j]].z);
        }
#if 0
        printf("Group %d/%d %d vertices\n",i ,self->n_groups-1, group->n_vertices);
        for(GLuint j = 0; j < group->n_vertices; j++){
            printf("#%d: %0.5f %0.5f %0.5f\n",
                   j, group->verts[group->indices[j]].x,
                   group->verts[group->indices[j]].y,
                   group->verts[group->indices[j]].z);
            printf("#%d: %0.5f %0.5f\n",
                j,
                group->texs[group->indices[j]].x,
                group->texs[group->indices[j]].y
            );
        }
#endif
    }
    printf("Mesh dumped\n");

}




Mesh *load_terrain(const char *filename)
{
    Mesh *rv = NULL;
    SGBinObject *terrain;
    int printed = 0;

    terrain = sg_bin_object_new();
    sg_bin_object_load(terrain, filename);
//    printf("Terrain version: %d\n",terrain->version);
#if 0
    if(!strcmp(filename,"/home/samuel/dev/Terrain/e000n40/e005n45/3039691.btg"))
        sg_bin_object_write_obj(terrain, "/tmp/cversion.obj");
#endif
//    printf("Doing %s, triangle_count: %d\n", filename,terrain->tris_v->len);
#if 0
    const double *llh;
    double xyz[3] = {terrain->gbs_center.x,
        terrain->gbs_center.y,
        terrain->gbs_center.z
    };

    llh = xyzllh(xyz);
    versor hlOr;
    versor p,q;

    glm_quat_from_lon_lat(p, llh[1], llh[0]);
    glm_quat_from_euler(q, 0, 0, glm_rad(180));

    glm_quat_mul(p, q, hlOr);
#endif

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

   //     printf("Found %d groups\n", ngroups);

        rv = mesh_new(ngroups);
        size_t g_idx = 0;
        start = 0;
        end = 1;
        glm_translated(rv->transformation, (vec3d){terrain->gbs_center.x, terrain->gbs_center.y, terrain->gbs_center.z});

      //  printf("tri_materials.size: %d\n", terrain->tri_materials->len);
        while ( start < terrain->tri_materials->len ) {
            // find next group
            material = g_ptr_array_index(terrain->tri_materials,start);
            while ( (end < terrain->tri_materials->len) &&
                    (!strcmp(material, g_ptr_array_index(terrain->tri_materials,end))) )
            {
           //     printf("end = %d\n",end);
                end++;
            }
        //    printf("group = %d to %d\n",start, end-1);


            // write group headers
//            printf("\n");
         //   printf("# usemtl %s\n", material);
            // write groups
            g_idx = mesh_get_vgroup_idx(rv, material, (end-start)*3);

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

           //     printf("\tTriangle %d: ",i);
                if(tri_v->len != 3)
                    printf("WARNING TRI_V IS NOT 3 !!\n");
                for (guint j = 2; j < tri_v->len; ++j ) { //Edges of the triangle
                    size_t idx;

                    int a3, b3;
                    GArray *ttcs3 = g_ptr_array_index(tri_tcs,0);
                    if(terrain->version >= 10){
                        a3 = g_array_index(tri_v, int, j);
                        b3 = g_array_index(ttcs3, int, j);
                    }else{
                        a3 = g_array_index(tri_v, uint16_t, j);
                        b3 = g_array_index(ttcs3, uint16_t, j);
                    }
                    SGVec3d vert3 = g_array_index(terrain->wgs84_nodes, SGVec3d, a3);
                    SGVec2f tex3 = g_array_index(terrain->texcoords, SGVec2f, b3);

                    int a2, b2;
                    GArray *ttcs2 = g_ptr_array_index(tri_tcs,0);
                    if(terrain->version >= 10){
                        a2 = g_array_index(tri_v, int, j - 1);
                        b2 = g_array_index(ttcs2, int, j - 1);
                    }else{
                        a2 = g_array_index(tri_v, uint16_t, j - 1);
                        b2 = g_array_index(ttcs2, uint16_t, j - 1);
                    }
                    SGVec3d vert2 = g_array_index(terrain->wgs84_nodes, SGVec3d, a2);
                    SGVec2f tex2 = g_array_index(terrain->texcoords, SGVec2f, b2);

                    int a1, b1;
                    GArray *ttcs1 = g_ptr_array_index(tri_tcs,0);
                    if(terrain->version >= 10){
                        a1 = g_array_index(tri_v, int, j - 2);
                        b1 = g_array_index(ttcs1, int, j - 2);
                    }else{
                        a1 = g_array_index(tri_v, uint16_t, j - 2);
                        b1 = g_array_index(ttcs1, uint16_t, j - 2);
                    }
                    SGVec3d vert1 = g_array_index(terrain->wgs84_nodes, SGVec3d, a1);
                    SGVec2f tex1 = g_array_index(terrain->texcoords, SGVec2f, b1);

                    vgroup_add_triangle(&(rv->groups[g_idx]),
                        &vert1, &tex1,
                        &vert2, &tex2,
                        &vert3, &tex3
                    );
                    //printf("%d ",a);

                }
                //printf("\n");
            }
//            printf("Group %d, final number of vertices %d vs %d\n", g_idx, rv->groups[g_idx].nverts,rv->groups[g_idx].n_vertices);

            start = end;
            end = start + 1;
        }
    }
//    rv->terrain = terrain;
    sg_bin_object_free(terrain);
    int i;
    for(i = 0; i < rv->n_groups; i++){
        if(rv->groups[i].tex_name == NULL)
            break;
    }
    printf("Going from %d groups to %d\n", rv->n_groups, i);
    rv->n_groups = i;
    qsort(rv->groups, rv->n_groups, sizeof(VGroup), (__compar_fn_t)vgroup_compare);
    printf("Done loading terrain %s\n",filename);
//    mesh_dump_buffer(rv);
//    exit(0);
    return rv;
}

void mesh_render_buffer(Mesh *self, GLuint position, GLuint texcoords, GLuint u_mvp, mat4d vp)
{
    VGroup *group;
    mat4d mvp;
    mat4 mvpf;

    glm_mat4d_mul(vp, self->transformation, mvp);
    glm_mat4d_ucopyf(mvp, mvpf);
    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, mvpf[0]);

    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);

        glActiveTexture(GL_TEXTURE0 );
        glBindTexture(GL_TEXTURE_2D, group->tex_id);

        glEnableVertexAttribArray(position);
        glBindBuffer(GL_ARRAY_BUFFER, group->vertex_buffer);
        glVertexAttribPointer(
            position,
            3,
            GL_FLOAT,
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
        glDrawElements(GL_TRIANGLES, group->n_indices, INDICE_TYPE, 0);

        glDisableVertexAttribArray(position);
        glDisableVertexAttribArray(texcoords);
    }
//    printf("Mesh %p rendered\n",self);
}

