#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SDL_timer.h"
#include "bucket.h"
#include "misc.h"

// return the horizontal tile span factor based on latitude
static double sg_bucket_span( double l ) {
    if ( l >= 89.0 ) {
	return 12.0;
    } else if ( l >= 86.0 ) {
	return 4.0;
    } else if ( l >= 83.0 ) {
	return 2.0;
    } else if ( l >= 76.0 ) {
	return 1.0;
    } else if ( l >= 62.0 ) {
	return 0.5;
    } else if ( l >= 22.0 ) {
	return 0.25;
    } else if ( l >= -22.0 ) {
	return 0.125;
    } else if ( l >= -62.0 ) {
	return 0.25;
    } else if ( l >= -76.0 ) {
	return 0.5;
    } else if ( l >= -83.0 ) {
	return 1.0;
    } else if ( l >= -86.0 ) {
	return 2.0;
    } else if ( l >= -89.0 ) {
	return 4.0;
    } else {
	return 12.0;
    }
}



void sg_bucket_set(SGBucket *self, double dlon, double dlat)
{
    memset(self, 0, sizeof(SGBucket));
    //
    // latitude first
    //
    double span = sg_bucket_span( dlat );
    double diff = dlon - (double)(int)dlon;
    // cout << "diff = " << diff << "  span = " << span << endl;
    if ( (dlon >= 0) || (fabs(diff) < SG_EPSILON) ) {
        self->lon = (int)dlon;
    } else {
        self->lon = (int)dlon - 1;
    }

    // find subdivision or super lon if needed
    if ( span < SG_EPSILON ) {
        // polar cap
        self->lon = 0;
        self->x = 0;
    } else if ( span <= 1.0 ) {
        self->x = (int)((dlon - self->lon) / span);
    } else {
        if ( dlon >= 0 ) {
            self->lon = (int)( (int)(self->lon / span) * span);
        } else {
            // cout << " lon = " << lon 
            //  << "  tmp = " << (int)((lon-1) / span) << endl;
            self->lon = (int)( (int)((self->lon + 1) / span) * span - span);
            if ( self->lon < -180 ) {
                self->lon = -180;
            }
        }
        self->x = 0;
    }

    //
    // then latitude
    //
    diff = dlat - (double)(int)dlat;

    if ( (dlat >= 0) || (fabs(diff) < SG_EPSILON) ) {
        self->lat = (int)dlat;
    } else {
        self->lat = (int)dlat - 1;
    }
    self->y = (int)((dlat - self->lat) * 8);
}


SGBucket *sg_bucket_new(double dlon, double dlat)
{
    SGBucket *rv;

    rv = calloc(1, sizeof(SGBucket));
    if(rv){
        sg_bucket_set(rv, dlon, dlat);
    }
    return rv;
}


SGBucket *sg_bucket_copy(SGBucket *self)
{
    SGBucket *rv;
    rv = calloc(1, sizeof(SGBucket));
    if(rv){
        memcpy(rv, self, sizeof(SGBucket));
    }
    return rv;
}

void sg_bucket_free(SGBucket *self)
{
    sg_bucket_unload_mesh(self);
    free(self);
}

// Build the path name for this bucket
const char *sg_bucket_gen_base_path(SGBucket *self)
{
    // long int index;
    int top_lon, top_lat, main_lon, main_lat;
    char hem, pole;
    static char raw_path[256];

    top_lon = self->lon / 10;
    main_lon = self->lon;
    if ( (self->lon < 0) && (top_lon * 10 != self->lon) ) {
	top_lon -= 1;
    }
    top_lon *= 10;
    if ( top_lon >= 0 ) {
	hem = 'e';
    } else {
	hem = 'w';
	top_lon *= -1;
    }
    if ( main_lon < 0 ) {
	main_lon *= -1;
    }
    
    top_lat = self->lat / 10;
    main_lat = self->lat;
    if ( (self->lat < 0) && (top_lat * 10 != self->lat) ) {
	top_lat -= 1;
    }
    top_lat *= 10;
    if ( top_lat >= 0 ) {
	pole = 'n';
    } else {
	pole = 's';
	top_lat *= -1;
    }
    if ( main_lat < 0 ) {
	main_lat *= -1;
    }

    snprintf(raw_path, 256, "%c%03d%c%02d/%c%03d%c%02d",
	    hem, top_lon, pole, top_lat, 
	    hem, main_lon, pole, main_lat);

    return raw_path;
}

long int sg_bucket_gen_index(SGBucket *self)
{
    return ((self->lon + 180) << 14) + ((self->lat + 90) << 6) + (self->y << 3) + self->x;
}

const char *sg_bucket_gen_index_str(SGBucket *self)
{
	static char tmp[20];
	snprintf(tmp, 20, "%ld",
                 (((long)self->lon + 180) << 14) + ((self->lat + 90) << 6)
                 + (self->y << 3) + self->x);
	return tmp;
}

const char *sg_bucket_getfilename(SGBucket *self)
{
    static char rv[256];

    snprintf(rv, 256, "%s/%s.btg", sg_bucket_gen_base_path(self), sg_bucket_gen_index_str(self));
    
    return rv;
}


// return width of the tile in degrees
double sg_bucket_get_width(SGBucket *self)
{
    return sg_bucket_span( sg_bucket_get_center_lat(self) );
}


// return height of the tile in degrees
double sg_bucket_get_height(SGBucket *self)
{
    return SG_BUCKET_SPAN;
}


/**
 * @return the center lon of a tile.
 */
double sg_bucket_get_center_lon(SGBucket *self)
{
    double span = sg_bucket_span( self->lat + self->y / 8.0 + SG_HALF_BUCKET_SPAN );

    if ( span >= 1.0 ) {
        return self->lon + sg_bucket_get_width(self) / 2.0;
    } else {
        return self->lon + self->x * span + sg_bucket_get_width(self) / 2.0;
    }
}

/**
 * @return the center lat of a tile.
 */
double sg_bucket_get_center_lat(SGBucket *self)
{
    return self->lat + self->y / 8.0 + SG_HALF_BUCKET_SPAN;
}

double sg_bucket_get_highest_lat(SGBucket *self)
{
    unsigned char adjustedY = self->y;
    if (self->lat >= 0) {
        // tile is north of the equator, so we want the top edge. Add one
        // to y to achieve this.
        ++adjustedY;
    }
    
	return self->lat + (adjustedY / 8.0);
}


// return width of the tile in meters. This function is used by the
// tile-manager to estimate how many tiles are in the view distance, so
// we care about the smallest width, which occurs at the highest latitude.
double sg_bucket_get_width_m(SGBucket *self)
{
    double clat_rad = sg_bucket_get_highest_lat(self) * (M_PI/180.0);
    double cos_lat = cos( clat_rad );
    if (fabs(cos_lat) < SG_EPSILON) {
        // happens for polar tiles, since we pass in a latitude of 90
        // return an arbitrary small value so all tiles are loaded
        return 10.0;
    }
    
    double local_radius = cos_lat * SG_EQUATORIAL_RADIUS_M;
    double local_perimeter = local_radius * (M_PI*2);
    double degree_width = local_perimeter / 360.0;

    return sg_bucket_get_width(self) * degree_width;
}

SGBucket *sg_bucket_sibling(SGBucket *self, int dx, int dy)
{
    
    double clat = sg_bucket_get_center_lat(self) + dy * SG_BUCKET_SPAN;
    // return invalid here instead of clipping, so callers can discard
    // invalid buckets without having to check if it's an existing one
    if ((clat < -90.0) || (clat > 90.0)) {
        return NULL;
    }
    
    // find the lon span for the new latitude
    double span = sg_bucket_span( clat );
    
    double tmp = sg_bucket_get_center_lon(self) + dx * span;
    tmp = normalize_periodicd(-180.0, 180.0, tmp);
    
    return sg_bucket_new(tmp, clat);
}

void sg_bucket_sibling_set(SGBucket *self, SGBucket *dest, int dx, int dy)
{
    
    double clat = sg_bucket_get_center_lat(self) + dy * SG_BUCKET_SPAN;
    // return invalid here instead of clipping, so callers can discard
    // invalid buckets without having to check if it's an existing one
    if ((clat < -90.0) || (clat > 90.0)) {
        memset(dest, 0, sizeof(SGBucket));
    }
    
    // find the lon span for the new latitude
    double span = sg_bucket_span( clat );
    
    double tmp = sg_bucket_get_center_lon(self) + dx * span;
    tmp = normalize_periodicd(-180.0, 180.0, tmp);
    
    sg_bucket_set(dest, tmp, clat);
}



Mesh *sg_bucket_get_mesh(SGBucket *self)
{
    char buffer[256];
    Uint32 start,end;

//    printf("Getting mesh for tile %p\n", self);
    if(!self->mesh){
        snprintf(buffer, 256, TERRAIN_ROOT"/%s", sg_bucket_getfilename(self));
        printf("Bucket %p: Will load next bucket: path=%s\n",self, buffer);
        start = SDL_GetTicks();
        self->mesh = mesh_new_from_file(buffer);
        end = SDL_GetTicks();
        printf("Mesh loaded from disk in %d ms\n",end-start);

    }
    return self->mesh;
}

void sg_bucket_unload_mesh(SGBucket *self)
{
    if(self->mesh){
        mesh_free(self->mesh);
        self->mesh = NULL;
    }
}

#if ENABLE_TEST
int main(int argc, char *argv[])
{
    SGBucket *b;
    double positions[2][2] = {
        {42.02597, 2.23869},
        {41.99979, 2.23788}
    };
//Current position: lat: 42.02597, lng: 2.23869, altitude: 2474.38 ft
    //lat: 41.99979, lng: 2.23788
    for(int i = 0; i < 2; i++){
        b = sg_bucket_new(positions[i][1], positions[i][0]);
        printf("%d: Path: %s\n",i,sg_bucket_gen_base_path(b));
        printf("%d: Index: %s\n",i,sg_bucket_gen_index_str(b));
        sg_bucket_free(b);
    }
	exit(EXIT_SUCCESS);
}
#endif
