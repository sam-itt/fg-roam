#ifndef SG_SPHERE_H
#define SG_SPHERE_H

#include <stdbool.h>

#include "sg-vec.h"

typedef struct {
    SGVec3d center;
    double radius;
} SGSphered;

static inline bool sg_sphered_valid(SGSphered *self)
{
    return 0 <= self->radius;
}

static inline bool sg_sphered_empty(SGSphered *self)
{
    return !sg_sphered_valid(self);
}


void sg_sphered_expand_by(SGSphered *self, SGVec3d *v);
#endif /* SG_SPHERE_H */
