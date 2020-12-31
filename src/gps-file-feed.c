#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gps-file-feed.h"

extern unsigned int global_accelerator;

static void gps_file_feed_get_next(GpsFileFeed *self, GpsRecord *record);

GpsFileFeed *gps_file_feed_new(void)
{
    GpsFileFeed *rv;

    rv = calloc(1, sizeof(GpsFileFeed));
    if(rv){
        GPS_FEED(rv)->get_next = (GetNextFunc)gps_file_feed_get_next;
    }
    return rv;
}


GpsFileFeed *gps_file_feed_new_from_file(const char *filename, int alt_offset)
{
    GpsFileFeed *rv;
    rv = gps_file_feed_new();
    GPS_FEED(rv)->alt_offset = alt_offset;
    if(!gps_file_feed_load_trace(rv, filename)){
        gps_file_feed_free(rv);
        rv = NULL;
    }
    return rv;
}

void gps_file_feed_free(GpsFileFeed *self)
{
   if(self->trace.records)
       free(self->trace.records);
   free(self);
}

bool gps_file_feed_load_trace(GpsFileFeed *self, const char *filename)
{
    FILE *fp;
    size_t fsize;

    fp = fopen(filename, "rb");
    if(!fp)
        return false;
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);  /* same as rewind(fp); */

    /* This will support allocating from a newly allocted object (nrecords == 0)
     * or clearing while re-allocating as needed if the object has been used before
     * */
    if(self->trace.nrecords < fsize / sizeof(GpsRecord)){
        self->trace.nrecords = fsize / sizeof(GpsRecord);
        self->trace.records = realloc(self->trace.records, self->trace.nrecords*sizeof(GpsRecord));
        if(!self->trace.records)
            exit(EXIT_FAILURE);
    }
    memset(self->trace.records, 0, self->trace.nrecords*sizeof(GpsRecord));

    fread(self->trace.records, sizeof(GpsRecord), self->trace.nrecords, fp);
    GpsRecord *current;
    for(size_t i = 0; i < self->trace.nrecords; i++){
        current = &(self->trace.records[i]);
        current->alt += GPS_FEED(self)->alt_offset;
    }
    fclose(fp);
    return true;
}

/* The following function assumes records are sorted (chrono)*/
void gps_file_feed_get(GpsFileFeed *self, GpsRecord *record, double dt)
{
    GpsRecord *before, *after;
    size_t i;
    time_t rdt;
    time_t relative_dt;
    static int last_idx = 0;

    for(i = last_idx; i < self->trace.nrecords; i++){
        relative_dt = self->trace.records[i].time - self->trace.records[0].time;
//        printf("relative dt of record %d is %ld\n",i,relative_dt);
        if(dt <= relative_dt){
//            printf("dt is %f, stopping at record %d with time %ld from start\n", dt, i, relative_dt);
            break;
        }
    }
    last_idx = i;
    if(i > 0 && dt < relative_dt){
        before = &self->trace.records[i-1];
        after = &self->trace.records[i];

//        printf("Interpolating between record %d, time %ld from start:\n", i-1, before->time - self->trace.records[0].time);
//        gps_record_dump(before);
//        printf("and record %d, time %ld from start:\n", i, after->time - self->trace.records[0].time);
//        gps_record_dump(after);

        rdt = after->time - before->time;
//        printf("dt between records is %ld\n",rdt);
        dt = dt - (before->time - self->trace.records[0].time);
        record->lat = before->lat + ((after->lat - before->lat)/rdt) * dt;
        record->lon = before->lon + ((after->lon - before->lon)/rdt) * dt;
        record->alt = before->alt + ((after->alt - before->alt)/rdt) * dt;
        record->time = before->time + dt;
    }else{
        *record = self->trace.records[i];
    }
//    printf("returning:\n");
//    gps_record_dump(record);
//    printf("\n");
}

static void gps_file_feed_get_next(GpsFileFeed *self, GpsRecord *record)
{
    static GpsRecord *current = NULL;
    static size_t current_idx = 0;
    static time_t last_ticks = 0;

    time_t rec_dt;
    time_t real_dt;

    if(current){
        rec_dt = self->trace.records[current_idx+1].time - current->time;
        real_dt = time(NULL) - last_ticks;
//        printf("Record dt: %ld, real dt: %ld, time to go: %ld\n",rec_dt, real_dt, real_dt-rec_dt);
//        real_dt *= global_accelerator;
        if(real_dt >= rec_dt){
            current_idx++;
            current = &(self->trace.records[current_idx]);
            last_ticks = time(NULL);
        }
    }else{
        current = &(self->trace.records[current_idx]);
        last_ticks = time(NULL);
    }
    *record = *current;
//    gps_record_dump(record);
//    printf("%s: returning idx %d\n", __FUNCTION__ ,current_idx);
}
