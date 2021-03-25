/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef GPS_FILE_FEED_H
#define GPS_FILE_FEED_H
#include <stdbool.h>

#include "gps-feed.h"

typedef struct{
    size_t nrecords;
    GpsRecord *records;
}GpsTrace;


typedef struct{
    GpsFeed parent;

    GpsTrace trace;
}GpsFileFeed;

#define gps_trace_relative_time(self, ridx) ((self)->trace.records[(ridx)].time -  (self)->trace.records[0].time)


GpsFileFeed *gps_file_feed_new_from_file(const char *filename, int alt_offset);
void gps_file_feed_free(GpsFileFeed *self);

bool gps_file_feed_load_trace(GpsFileFeed *self, const char *filename);
void gps_file_feed_get(GpsFileFeed *self, GpsRecord *record, double dt);
#endif
