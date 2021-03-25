/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <float.h>
#include <math.h>

#include "sg-vec.h"

void sg_vect3f_normalize(SGVec3f *self)
{

    float normv = sqrt((self->x*self->x) + (self->y*self->y) + (self->z*self->z));
    if(normv <= FLT_MIN)
        *self = (SGVec3f){0.0, 0.0, 0.0};
    else
        *self = (SGVec3f){1/normv*self->x,1/normv*self->y,1/normv*self->z};
}

double sg_vect3d_distSqr(SGVec3d *a, SGVec3d *b)
{
    SGVec3d tmp;

    tmp = (SGVec3d){
        .x = a->x - b->x,
        .y = a->y - b->y,
        .z = a->z - b->z,
    };

    return (tmp.x*tmp.x) + (tmp.y*tmp.y) + (tmp.z*tmp.z);
}

