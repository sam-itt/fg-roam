/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef VERTEX_SET_H
#define VERTEX_SET_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "indice.h"
#include "sg-vec.h"

typedef struct _IndexedVertex{
    /* 'virtual' index of the vertex presented to outside
     * as if all vertices where together side by side
     */
    indice_t index;

    /*vertex properties*/
    SGVec3d position;
    SGVec2f texcoords;

    struct _IndexedVertex *next;
}IndexedVertex;

typedef struct{
    /*too big?*/
    indice_t size; /*can hold up to size first-level elements*/
    /* number of elements, not necessarily consecutive*/
    indice_t nelements;

    IndexedVertex *vertices;
}VertexSet;


VertexSet *vertex_set_new(size_t size);
VertexSet *vertex_set_init(VertexSet *self, size_t size);

VertexSet *vertex_set_dispose(VertexSet *self);
VertexSet *vertex_set_free(VertexSet *self);

IndexedVertex *vertex_set_add_vertex(VertexSet *self,
                                     SGVec3d *position,
                                     SGVec2f *texcoords);

IndexedVertex *vertex_set_get_vertex(VertexSet *self,
                                     SGVec3d *position,
                                     SGVec2f *texcoords);

bool vertex_set_flatten(VertexSet *self, indice_t *nvertices,
                        SGVec3f **positions, SGVec2f **texcoords);
#endif /* VERTEX_SET_H */
