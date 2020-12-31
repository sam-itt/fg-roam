#ifndef GPS_FEED_H
#define GPS_FEED_H

#include <time.h>

typedef struct{
    time_t time;
    double lat; /*dec degrees*/
    double lon; /*dec degrees*/
    double alt; /*Alt meters*/
}__attribute__((packed)) GpsRecord;

#define GPS_RECORD_INVALID (GpsRecord){0, NAN, NAN, NAN}
#define gps_record_equals(a,b) (((a)->time == (b)->time) && ((a)->lat == (b)->lat) && ((a)->lon == (b)->lon) && ((a)->alt == (b)->alt))

typedef struct _GpsFeed GpsFeed;
typedef void (*GetNextFunc)(GpsFeed *self, GpsRecord *record);

struct _GpsFeed{
    int alt_offset;

    GetNextFunc get_next;
};

#define GPS_FEED(a) ((GpsFeed *)(a))

#define gps_feed_get_next(self, record) ((self)->get_next((self), (record)))
#if 0
inline void gps_feed_get_next(GpsFeed *self, GpsRecord *record)
{
    return self->get_next(self, record);
}
#endif


void gps_record_dump(GpsRecord *self);
#endif
