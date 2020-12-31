#ifndef TILE_MANAGER_H
#define TILE_MANAGER_H

#include "bucket.h"
#define MAX_BUCKETS 20

typedef struct{
    SGBucket *buckets[MAX_BUCKETS];
//    int n_buckets;

}TileManager;


TileManager *tile_manager_get_instance(void);
void tile_manager_shutdown(void);


SGBucket **tile_manager_get_tiles(TileManager *self, double lat, double lon, double vis);
SGBucket *tile_manager_get_tile_t(TileManager *self, SGBucket *t);
SGBucket *tile_manager_get_tile(TileManager *self, double lat, double lon);
bool tile_manager_add_tile(TileManager *self, SGBucket *bucket);
SGBucket *tile_manager_add_tile_copy(TileManager *self, SGBucket *bucket);
#endif
