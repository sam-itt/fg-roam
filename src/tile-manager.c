/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
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
    size_t add_idx;

    if(self->nbuckets == MAX_BUCKETS){
        /* If we get here, we already have all buckets slots used:
         * remove the older
         */
        Uint32 oldest_stamp = SDL_GetTicks(); /*start at now() and look for the smallest value*/
        for(int i = 0; i < self->nbuckets; i++){
            if(oldest_stamp > self->buckets[i]->last_used){
                oldest_stamp = self->buckets[i]->last_used;
                add_idx = i;
            }
        }
        sg_bucket_free(self->buckets[add_idx]);
    }else{
        add_idx = self->nbuckets;
        self->nbuckets++;
    }

    self->buckets[add_idx] = bucket;
    return true;
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

SGBucket *tile_manager_get_tile(TileManager *self, double lat, double lon)
{
    SGBucket *rv;
    SGBucket tmp;

    sg_bucket_set(&tmp, lon, lat);
    /*TODO: Use a hash-like structure?*/
    for(int i = 0; i < self->nbuckets; i++){
        if(!self->buckets[i]) continue;
        if(sg_bucket_equals(&tmp, self->buckets[i])){
            self->buckets[i]->last_used = SDL_GetTicks();
            return self->buckets[i];
        }
    }

    rv = tile_manager_add_tile_copy(self, &tmp);
    if(!rv){
        printf("Couldn't add tile, bailing out\n");
        exit(EXIT_FAILURE);
    }
    rv->last_used = SDL_GetTicks();
    return rv;
}

/*return next index*/
static size_t add_bucket(size_t nbuckets, size_t abuckets, SGBucket **buckets, SGBucket *candidate)
{
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
    static SGBucket *rv[6];
    SGBucket *tmp;
    bool found;
    GeoLocation nbox[2];
    int nbuckets;

    geo_location_bounding_coordinates(location, vis, nbox);

    nbuckets = 0;
    //center
    tmp = tile_manager_get_tile(self, location->latitude, location->longitude);
    nbuckets = add_bucket(nbuckets, 4, rv, tmp);
#if !defined (NO_PRELOAD) || NO_PRELOAD == 0
    //up left
    tmp = tile_manager_get_tile(self, nbox[1].latitude, nbox[0].longitude); //lat lon, Y X
    nbuckets = add_bucket(nbuckets, 4, rv, tmp);
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
#endif
    rv[nbuckets] = NULL;

/*    printf("Bounding locations for region %f m around lat: %f, lon: %f:\n"*/
            /*"\tTile 0: lat:%d lon:%d x:%d y:%d\n"*/
            /*"\tTile 1: lat:%d lon:%d x:%d y:%d\n"*/
            /*"\tTile 2: lat:%d lon:%d x:%d y:%d\n"*/
            /*"\tTile 3: lat:%d lon:%d x:%d y:%d\n",*/
        /*vis,*/
        /*location->latitude, location->longitude,*/
        /*rv[0]->lat, rv[0]->lon, rv[0]->x, rv[0]->y,*/
        /*rv[1]->lat, rv[1]->lon, rv[1]->x, rv[1]->y,*/
        /*rv[2]->lat, rv[2]->lon, rv[2]->x, rv[2]->y,*/
        /*rv[3]->lat, rv[3]->lon, rv[3]->x, rv[3]->y*/
    /*);*/
//    fflush(stdout);


    return rv;
}

