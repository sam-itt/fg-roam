/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "vertex-set.h"

#define position_equals(p1, p2) (((p1)->x == (p2)->x) && ((p1)->y == (p2)->y) && ((p1)->z == (p2)->z))
#define texcoords_equals(t1, t2) ((t1)->x == (t2)->x) && ((t1)->y == (t2)->y)

static uint32_t indexed_vertex_hashcode(SGVec3d *position, SGVec2f *texcoords);
static inline bool indexed_vertex_equals(IndexedVertex *self, SGVec3d *position, SGVec2f *texcoords);

/**
 * VertexSet: A set of vertices that only stores a given vertex
 * (distinct set of position, texture coordinates, etc.) once.
 *
 *
 */

/**
 * @brief Creates a new VertexSet.
 *
 * Caller must free the VertexSet using vertex_set_free when done.
 *
 * @param size A hint on the amount of vertex the set is going to hold.
 * The VertexSet will be able to store more but less efficiently.
 * @return a newly allocated VertexSet or NULL
 * on failure.
 *
 * @see vertex_set_free
 */
VertexSet *vertex_set_new(size_t size)
{
    VertexSet *self;

    self = calloc(1, sizeof(VertexSet));
    if(self){
        if(!vertex_set_init(self, size))
            return vertex_set_free(self);
    }
    return self;
}

/**
 * @brief Inits a VertexSet.
 *
 * Caller must dispose the VertexSet using vertex_set_dispose when done.
 *
 * @param size A hint on the amount of vertex the set is going to hold.
 * The VertexSet will be able to store more but less efficiently.
 * @return @p self on success or NULL
 * on failure.
 *
 * @see vertex_set_dispose
 */
VertexSet *vertex_set_init(VertexSet *self, size_t size)
{
    self->size = size;
    self->vertices = calloc(self->size, sizeof(IndexedVertex));
    if(!self->vertices)
        return NULL;
    for(int i = 0; i < self->size; i++)
        self->vertices[i].position.x = NAN;

    return self;
}

VertexSet *vertex_set_dispose(VertexSet *self)
{
    IndexedVertex *current, *next;

    for(int i = 0; i < self->size; i++){
        if(isnan(self->vertices[i].position.x)) continue;
        /* All 'next' IndexedVertex have been allocated,
         * only self->vertices[x] is part of the array */
        current = self->vertices[i].next;
        while(current){
            next = current->next;
            free(current);
            current = next;
        }
    }
    if(self->vertices)
        free(self->vertices);

    return NULL;
}

VertexSet *vertex_set_free(VertexSet *self)
{
    vertex_set_dispose(self);
    free(self);
    return NULL;
}

/**
 * @brief Ensures that a vertex with given properties is present in the set.
 *
 * This function will either return an existing vertex with matching properties
 * or a newly added vertex either case the indice can be used. If it fails it will
 * return NULL.
 *
 * @param self a VertexSet
 * @param position pointer to a SGVec3d which represents the vertex's position
 * @param texcoords pointer to a SGVec2f which represents the vertex's u/v
 * texture coordinates
 * @return a IndexedVertex on success, NULL on failure (memory).
 */
IndexedVertex *vertex_set_add_vertex(VertexSet *self,
                                     SGVec3d *position,
                                     SGVec2f *texcoords)
{
    IndexedVertex *iv, *prev;
    size_t idx;

    idx = indexed_vertex_hashcode(position, texcoords);
    idx = idx % self->size;

    if(!isnan(self->vertices[idx].position.x)){/*There is already something for this idx*/
        /*Going to the last element. */
        for(iv = &self->vertices[idx]; iv != NULL; iv = iv->next){
            if(indexed_vertex_equals(iv, position, texcoords)){
                return iv;
            }
            prev = iv;
        }
        /* If we get here, we had at least an existing vertex for that hash
         * but for a different vertex. We need to chain up
         * next to it
         * */
        prev->next = calloc(1, sizeof(IndexedVertex));
        if(!prev->next)
            return NULL;
        iv = prev->next;
    }else{/*Slot is free*/
        iv = &self->vertices[idx];
    }
    *iv = (IndexedVertex){
        .index = self->nelements++,
        .position = *position,
        .texcoords = *texcoords
    };

    return iv;
}

IndexedVertex *vertex_set_get_vertex(VertexSet *self,
                                     SGVec3d *position,
                                     SGVec2f *texcoords)
{
    IndexedVertex *iv;
    size_t idx;

    idx = indexed_vertex_hashcode(position, texcoords);
    idx = idx % self->size;

    if(!isnan(self->vertices[idx].position.x)){/*There is already something for this idx*/
        /*Going to the last element. */
        for(iv = &self->vertices[idx]; iv != NULL; iv = iv->next){
            if(indexed_vertex_equals(iv, position, texcoords)){
                return iv;
            }
        }
    }
    return NULL;
}

/**
 * @brief Flatten stored vertex data into arrays while preserving
 * indices.
 *
 * Allocates needed memory for each vertex attribute (currently
 * position and texture coordinates). Calling code becomes responsible
 * for freeing the memory by calling free(3) on it.
 *
 * @param self a VertexSet
 * @param nvertices a place to store the number of vertices in the arrays.
 * @param positions a place to store the adress of the newly allocated array.
 * @param textcoords a place to store the adress of the newly allocated array.
 * @return true on success, false on failure.
 *
 * @note: If the function fails the caller doesn't need to free
 * any of pointers. Ownership is only transfered on success
 */
bool vertex_set_flatten(VertexSet *self, indice_t *nvertices,
                        SGVec3f **positions, SGVec2f **texcoords)
{
    IndexedVertex *iv;

    *positions = malloc(sizeof(SGVec3f)*self->nelements);
    *texcoords = malloc(sizeof(SGVec2f)*self->nelements);
    if(!*positions || !*texcoords)
        goto bail;
    *nvertices = self->nelements;
    for(int i = 0; i < self->size; i++){
        if(isnan(self->vertices[i].position.x)) continue;
        for(iv = &self->vertices[i]; iv != NULL; iv = iv->next){
            (*positions)[iv->index] = (SGVec3f){
                .x = iv->position.x,
                .y = iv->position.y,
                .z = iv->position.z
            };
            (*texcoords)[iv->index] = iv->texcoords;
        }
    }
    return true;

bail:
    if(*positions)
        free(*positions);
    if(*texcoords)
        free(*texcoords);
    return false;
}

static uint32_t indexed_vertex_hashcode(SGVec3d *position, SGVec2f *texcoords)
{
    int rv = 23; /*any prime will do*/

    /*same for 31*/
    rv = rv * 31 + position->x;
    rv = rv * 31 + position->y;
    rv = rv * 31 + position->z;

    rv = rv * 31 + texcoords->x;
    rv = rv * 31 + texcoords->y;

    return rv;
}

static inline bool indexed_vertex_equals(IndexedVertex *self,
                                         SGVec3d *position,
                                         SGVec2f *texcoords)
{
    if(position_equals(&self->position, position)
       && texcoords_equals(&self->texcoords, texcoords)){
        return true;
    }

    return false;
}
