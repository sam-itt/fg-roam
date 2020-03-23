#define _GNU_SOURCE
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
#include "misc.h"

#define OPTIMIZE_INDICES 1
#define GROUP_VERTICES 1

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
    self->indices = calloc(size, sizeof(GLuint));
//    printf("allocated %d indices at %p\n",size,self->indices);
    self->n_vertices = size;
    self->sbound.radius = -1.0;

    self->global_lookup = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);

    return self;
}



Mesh *mesh_new(size_t size, GLuint nverts )
{
    Mesh *rv;
    rv = calloc(1, sizeof(Mesh));
    if(rv){
        rv->groups = calloc(size, sizeof(VGroup));
        rv->n_groups = size;

        rv->vert_esize = nverts;
        rv->nverts = nverts;
        rv->verts = calloc(rv->nverts, sizeof(SGVec3d));
        rv->texs = calloc(rv->nverts, sizeof(SGVec2f));

        rv->n_vertices = nverts;

        rv->stats = g_hash_table_new_full(g_str_hash, g_str_equal,g_free, (GDestroyNotify)g_array_unref);
        rv->global_lookup = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, NULL);
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
        free(group->verts);
        free(group->texs);
        free(group->indices);
        g_hash_table_unref(group->global_lookup);
    }
    free(self->groups);
    free(self->verts);
    free(self->texs);
    g_hash_table_unref(self->stats);
    g_hash_table_unref(self->global_lookup);
    free(self);
}

size_t mesh_get_size(Mesh *self, bool data_only)
{
    VGroup *group;
    size_t rv;

    rv = 0;

#if !GROUP_VERTICES
    rv += sizeof(SGVec3d)*self->nverts;
    rv += sizeof(SGVec2f)*self->nverts;
#endif
    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        if(!data_only)
           rv += sizeof(VGroup);
#if GROUP_VERTICES
#if OPTIMIZE_INDICES
        rv += sizeof(SGVec3d) * group->nverts; //verts
        rv += sizeof(SGVec2f) * group->nverts; //tex
#else
        rv += sizeof(SGVec3d) * group->n_vertices; //verts
        rv += sizeof(SGVec2f) * group->n_vertices; //tex
#endif
#endif
        rv += sizeof(GLuint) * group->n_vertices;
    }

    if(!data_only)
        rv += sizeof(Mesh);
    return rv;
}

void mesh_register_vertex_use(Mesh *self, guint group, guint pos_idx, guint tex_idx)
{
    GArray *array;
    gchar *key;
    guint current;
    bool nofree = false;

    key = g_strdup_printf("%d-%d",pos_idx,tex_idx);
    array = g_hash_table_lookup(self->stats,key);
    if(!array){
        array = g_array_sized_new(FALSE, TRUE, sizeof(guint), self->n_groups);
        g_hash_table_insert(self->stats, key, array);
        nofree = true;
    }
    current = g_array_index(array, guint, group);
    current++;
    g_array_insert_val(array, group, current);
    if(!nofree)
        g_free(key);
}

void dofun( gchar *key, GArray *value, Mesh *self)
{
    int usage_count = 0;
    for(GLuint i = 0; i < self->n_groups; i++){
        guint guse;
        guse = g_array_index(value, guint, i);
        if(guse >= 1)
            usage_count++;
    }
    if(usage_count > 1){
        printf("Vertex %s referenced by %d groups\n", key, usage_count);
    }
}

void mesh_show_vertex_use(Mesh *self)
{
    g_hash_table_foreach(self->stats,(GHFunc) dofun, self);
}

Mesh *mesh_prepare(Mesh *self)
{

    VGroup *group;

#if !GROUP_VERTICES
    glGenBuffers(1, &(self->vertex_buffer));
    glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        self->nverts*sizeof(SGVec3d),
        self->verts,
        GL_STATIC_DRAW
    );
    glGenBuffers(1, &(self->texs_buffer));
    glBindBuffer(GL_ARRAY_BUFFER, self->texs_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        self->nverts*sizeof(SGVec2f),
        self->texs,
        GL_STATIC_DRAW
    );
#endif
    for(unsigned int i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
#if GROUP_VERTICES
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
#endif
        glGenBuffers(1, &(group->element_buffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group->element_buffer);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, 
            group->n_vertices*sizeof(GLuint),
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
#if GROUP_VERTICES
            printf("#%d: %0.5f %0.5f %0.5f\n",j, group->verts[group->indices[j]].x, group->verts[group->indices[j]].y, group->verts[group->indices[j]].z);
#else
            printf("#%d: %0.5f %0.5f %0.5f\n",j, self->verts[group->indices[j]].x, self->verts[group->indices[j]].y, self->verts[group->indices[j]].z);
#endif
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


size_t mesh_add_vertex(Mesh *self, SGVec3d *v, SGVec2f *tex, int global_idx)
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
        self->texs = reallocarray(self->texs,  self->vert_esize, sizeof(SGVec2f));
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


        rv = mesh_new(ngroups, terrain->wgs84_nodes->len);
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
                    GArray *ttcs = g_ptr_array_index(tri_tcs,0);
                    a = g_array_index(tri_v, int, j);
                    b = g_array_index(ttcs, int, j);
                    SGVec3d vert = g_array_index(terrain->wgs84_nodes, SGVec3d, a);
                    SGVec2f tex = g_array_index(terrain->texcoords, SGVec2f, b);
                    vert.x += terrain->gbs_center.x;
                    vert.y += terrain->gbs_center.y;
                    vert.z += terrain->gbs_center.z;

//                    mesh_register_vertex_use(rv, g_idx, a, b);
#if GROUP_VERTICES
#if OPTIMIZE_INDICES
                    rv->groups[g_idx].indices[vidx] = vgroup_add_vertex(&(rv->groups[g_idx]), &vert, &tex,a);
#else
                    rv->groups[g_idx].texs[vidx].x = tex.x;
                    rv->groups[g_idx].texs[vidx].y = tex.y;

                    rv->groups[g_idx].verts[vidx].x = vert.x;
                    rv->groups[g_idx].verts[vidx].y = vert.y;
                    rv->groups[g_idx].verts[vidx].z = vert.z;

                    rv->groups[g_idx].indices[vidx] = vidx;
#endif
#else
                    rv->groups[g_idx].indices[vidx] = mesh_add_vertex(rv, &vert , &tex, a);
#endif
                    if(rv->groups[g_idx].indices[vidx] > rv->higher_indice_seen)
                        rv->higher_indice_seen = rv->groups[g_idx].indices[vidx];

                    sg_sphered_expand_by(&(rv->groups[g_idx].sbound), &vert);

                    vidx++;
                }
            }
            printf("Group %d, final number of vertices %d vs %d\n", g_idx, rv->groups[g_idx].nverts,rv->groups[g_idx].n_vertices);
#if !OPTIMIZE_INDICES
            rv->groups[g_idx].nverts = rv->groups[g_idx].n_vertices;
#endif
            g_idx++;

            start = end;
            end = start + 1;
        }
    }
//    rv->terrain = terrain;
    sg_bin_object_free(terrain);
    printf("Done loading terrain\n");
    printf("Higher indice seen: %d\n", rv->higher_indice_seen);
    return rv;
}

void mesh_render_buffer(Mesh *self, GLuint position, GLuint texcoords)
{
    VGroup *group;

    glPushMatrix();

#if !GROUP_VERTICES
    glEnableVertexAttribArray(position);
    glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer);
    glVertexAttribPointer(
        position,
        3,
        GL_DOUBLE,
        GL_FALSE,
        0, /*no need to specify stride on single attribute vectors*/
        (void*)0
    );

    glEnableVertexAttribArray(texcoords);
    glBindBuffer(GL_ARRAY_BUFFER, self->texs_buffer);
    glVertexAttribPointer(
        texcoords,
        2,
        GL_FLOAT,
        GL_FALSE,
        0, /*As above*/
        (void*)0
    );
#endif
    for(GLuint i = 0; i < self->n_groups; i++){
        group = &(self->groups[i]);
        glActiveTexture(GL_TEXTURE0 );
        glBindTexture(GL_TEXTURE_2D, group->tex_id);
#if GROUP_VERTICES
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
#endif
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, group->element_buffer);
        glDrawElements(GL_TRIANGLES, group->n_vertices, GL_UNSIGNED_INT, 0);
#if GROUP_VERTICES
        glDisableVertexAttribArray(position);
        glDisableVertexAttribArray(texcoords);
#endif
    }
//    printf("Mesh %p rendered\n",self);
    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(texcoords);
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

