#include <stdio.h>
#include <stdlib.h>

#include "gps-feed.h"

void gps_record_dump(GpsRecord *self)
{
    printf("GpsRecord %p: time: %ld, lat: %0.17g, lon: %0.17g, alt: %0.17g\n",self,self->time,self->lat,self->lon,self->alt);
}

