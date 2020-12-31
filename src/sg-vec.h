#ifndef SG_VEC_H
#define SG_VEC_H

typedef struct {
    double x;
    double y;
    double z;
} SGVec3d;

typedef struct {
    float x;
    float y;
    float z;
} SGVec3f;

typedef struct {
    float x;
    float y;
} SGVec2f;


typedef struct {
    float red;
    float green;
    float blue;
    float alpha;
} SGVec4f;

#define sg_vec3_equals(a, b) ((a)->x == (b)->x) && ((a)->y == (b)->y) && ((a)->z == (b)->z)

void sg_vect3f_normalize(SGVec3f *self);
double sg_vect3d_distSqr(SGVec3d *a, SGVec3d *b);
#endif /* SG_VEC_H */
