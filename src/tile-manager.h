#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include "bucket.h"
#include "geo-location.h"
#define MAX_BUCKETS 8

typedef struct{
    SGBucket *buckets[MAX_BUCKETS];
    size_t nbuckets;

}TileManager;


TileManager *tile_manager_get_instance(void);
void tile_manager_shutdown(void);


SGBucket **tile_manager_get_tiles(TileManager *self, GeoLocation *location, float vis);
SGBucket *tile_manager_get_tile(TileManager *self, double lat, double lon);
bool tile_manager_add_tile(TileManager *self, SGBucket *bucket);
SGBucket *tile_manager_add_tile_copy(TileManager *self, SGBucket *bucket);
#endif
