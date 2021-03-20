#ifndef BUCKET_H
#define BUCKET_H
#include <SDL2/SDL.h>

#include "mesh.h"

#define 	SG_EPSILON   0.0000001
/**
 * standard size of a bucket in degrees (1/8 of a degree)
 */
#define SG_BUCKET_SPAN      0.125

/**
 * half of a standard SG_BUCKET_SPAN
 */
#define SG_HALF_BUCKET_SPAN ( 0.5 * SG_BUCKET_SPAN )

#define SG_EQUATORIAL_RADIUS_M 6378138.12

typedef struct{
    short lon;        // longitude index (-180 to 179)
    short lat;        // latitude index (-90 to 89)
    unsigned char x;          // x subdivision (0 to 7)
    unsigned char y;          // y subdivision (0 to 7)

    Mesh *mesh;
    Uint32 last_used;
}SGBucket;

#define sg_bucket_equals(a,b) (((a)->lon == (b)->lon) && ((a)->lat == (b)->lat) && ((a)->x == (b)->x) && ((a)->y == (b)->y))

SGBucket *sg_bucket_new(double dlon, double dlat);
SGBucket *sg_bucket_copy(SGBucket *self);
void sg_bucket_set(SGBucket *self, double dlon, double dlat);
void sg_bucket_free(SGBucket *self);
SGBucket *sg_bucket_sibling(SGBucket *self, int dx, int dy);
void sg_bucket_sibling_set(SGBucket *self, SGBucket *dest, int dx, int dy);

const char *sg_bucket_gen_base_path(SGBucket *self);
long int sg_bucket_gen_index(SGBucket *self);
const char *sg_bucket_gen_index_str(SGBucket *self);
const char *sg_bucket_getfilename(SGBucket *self);

double sg_bucket_get_width(SGBucket *self);
double sg_bucket_get_height(SGBucket *self);
double sg_bucket_get_center_lon(SGBucket *self);
double sg_bucket_get_center_lat(SGBucket *self);
double sg_bucket_get_width_m(SGBucket *self);

Mesh *sg_bucket_get_mesh(SGBucket *self);
void sg_bucket_unload_mesh(SGBucket *self);


#endif
