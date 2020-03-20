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

TileManager *tile_manager_get_instance(void)
{
    if(!instance){
        instance = tile_manager_new();
    }
    return instance;
}

bool tile_manager_add_tile(TileManager *self, SGBucket *bucket)
{
     for(int i = 0; i < MAX_BUCKETS; i++){
        if(!self->buckets[i] || self->buckets[i]->active == false){
            if(self->buckets[i])
                sg_bucket_free(self->buckets[i]);
            self->buckets[i] = bucket;
            printf("Added tile %p: %s\n", bucket, sg_bucket_getfilename(bucket));
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


SGBucket **tile_manager_get_tiles(TileManager *self, double lat, double lon, double vis)
{
    static SGBucket *rv[5];
    SGBucket tmp;
    bool found;
    double *box;

     for(int i = 0; i < MAX_BUCKETS; i++){
        if(!self->buckets[i]) continue;
        self->buckets[i]->active = false;
     }
   
    box = geo_bounding_box(lat, lon, vis);
    /*box= 0:ul_y, 1:ul_x, 2:dr_y, 3:dr_x*/
    //up left
    rv[0] = tile_manager_get_tile(self, box[0], box[1]); //lat lon, Y X
    //down left
    rv[1] = tile_manager_get_tile(self, box[2], box[1]);
    //down right
    rv[2] = tile_manager_get_tile(self, box[2], box[3]);
    //up right
    rv[3] = tile_manager_get_tile(self, box[0], box[3]);
    rv[4] = NULL;

    for(int i = 0; i < 4; i++)
        rv[i]->active = true;
    return rv;
}

