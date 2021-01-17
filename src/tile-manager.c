#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tile-manager.h"
#include "geodesy.h"

static TileManager *instance = NULL;

static TileManager *tile_manager_new(void)
{
    TileManager *rv;

    rv = calloc(1, sizeof(TileManager));

    return rv;
}

static void tile_manager_free(TileManager *self)
{
    for(int i = 0; i < MAX_BUCKETS; i++){
        if(self->buckets[i])
            sg_bucket_free(self->buckets[i]);
    }
    free(self);
}

TileManager *tile_manager_get_instance(void)
{
    if(!instance){
        instance = tile_manager_new();
    }
    return instance;
}

void tile_manager_shutdown(void)
{
    if(instance){
       tile_manager_free(instance);
       instance = NULL;
    }

}

bool tile_manager_add_tile(TileManager *self, SGBucket *bucket)
{
     for(int i = 0; i < MAX_BUCKETS; i++){
        if(!self->buckets[i] || self->buckets[i]->active == false){
            if(self->buckets[i]){
                sg_bucket_free(self->buckets[i]);
            }
            self->buckets[i] = bucket;
            //printf("Added tile %p as index %d: %s\n", bucket, i, sg_bucket_getfilename(bucket));
            bucket->active = true;
            return true;
        }
     }
     return false;
}

SGBucket *tile_manager_add_tile_copy(TileManager *self, SGBucket *bucket)
{
    SGBucket *copy;

    copy = sg_bucket_copy(bucket);
    if(!tile_manager_add_tile(self,copy)){
        sg_bucket_free(copy);
        return NULL;
    }
    return copy;
}


SGBucket *tile_manager_get_tile_t(TileManager *self, SGBucket *t)
{
    SGBucket *rv;
    bool found;

    for(int i = 0; i < MAX_BUCKETS; i++){
        if(!self->buckets[i]) continue;
        if(sg_bucket_equals(t, self->buckets[i])){
            return self->buckets[i];
        }
    }

    rv = tile_manager_add_tile_copy(self, t);
    if(!rv){
        printf("Couldn't add tile, bailing out\n");
        exit(EXIT_FAILURE);
    }
    return rv;
}

SGBucket *tile_manager_get_tile(TileManager *self, double lat, double lon)
{
    SGBucket tmp;

    sg_bucket_set(&tmp, lon, lat);

    return tile_manager_get_tile_t(self, &tmp);
}

/*return next index*/
static size_t add_bucket(size_t nbuckets, size_t abuckets, SGBucket **buckets, SGBucket *candidate)
{
    candidate->active = true;
    for(int i = 0; i < nbuckets; i++){
        if(buckets[i] == candidate)
            return nbuckets;
    }

    //printf("Adding tile %p as bucket #%d\n", candidate, nbuckets);
    buckets[nbuckets++] = candidate;
    return nbuckets;
}
/**
 *
 * vis in m
 */
SGBucket **tile_manager_get_tiles(TileManager *self, GeoLocation *location, float vis)
{
    static SGBucket *rv[5];
    SGBucket *tmp;
    bool found;
    GeoLocation nbox[2];
    int nbuckets;

     for(int i = 0; i < MAX_BUCKETS; i++){
        if(!self->buckets[i]) continue;
        self->buckets[i]->active = false;
     }

    geo_location_bounding_coordinates(location, vis, nbox);

    //up left
    tmp = tile_manager_get_tile(self, nbox[1].latitude, nbox[0].longitude); //lat lon, Y X
    nbuckets = add_bucket(0, 4, rv, tmp);
    //buckets[0]->active = true;
    //down left
    tmp = tile_manager_get_tile(self, nbox[0].latitude, nbox[0].longitude);
    nbuckets = add_bucket(nbuckets, 4, rv, tmp);

    //down right
    tmp = tile_manager_get_tile(self, nbox[0].latitude, nbox[1].longitude);
    nbuckets = add_bucket(nbuckets, 4, rv, tmp);

    //up right
    tmp = tile_manager_get_tile(self, nbox[1].latitude, nbox[1].longitude);
    nbuckets = add_bucket(nbuckets, 4, rv, tmp);
    rv[nbuckets] = NULL;

    return rv;
}

