/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <math.h>

#include "sg-sphere.h"

void sg_sphered_expand_by(SGSphered *self, SGVec3d *v)
{
    if (sg_sphered_empty(self)) {
      self->center = *v;
      self->radius = 0;
      return;
    }

    double dist2 = sg_vect3d_distSqr(&(self->center), v);
    if (dist2 <= (self->radius*self->radius))
      return;

    double dist = sqrt(dist2);
    double newRadius = (double)(0.5)*(self->radius + dist);
//    self->center += ((newRadius - self->radius)/dist)*(v - self->center);
    SGVec3d delta_v = (SGVec3d){
        .x = v->x - self->center.x,
        .y = v->y - self->center.y,
        .z = v->z - self->center.z,
    };
    double factor = (newRadius - self->radius)/dist;
    self->center = (SGVec3d){
        .x = self->center.x + factor * delta_v.x,
        .y = self->center.y + factor * delta_v.y,
        .z = self->center.z + factor * delta_v.z,
    };
    self->radius = newRadius;
}
