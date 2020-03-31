#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gps-file-feed.h"

int main(int argc, char *argv[])
{
    GpsFileFeed *feed;
    GpsRecord r;

    feed = gps_file_feed_new_from_file("test.gps", 0);
    memset(&r, 0, sizeof(GpsRecord));

    gps_file_feed_get(feed, &r, 0.0);
    gps_file_feed_get(feed, &r, 1.0);
    gps_file_feed_get(feed, &r, 6.0);
    gps_file_feed_get(feed, &r, 8.0);
    gps_file_feed_get(feed, &r, 10.0);
    gps_file_feed_get(feed, &r, 16.0);
    gps_file_feed_get(feed, &r, 20.0);
    gps_file_feed_get(feed, &r, 65.0);
    gps_file_feed_get(feed, &r, 80.0);
    gps_file_feed_get(feed, &r, 4*60.0);
    gps_file_feed_get(feed, &r, 5*60.0);

	exit(EXIT_SUCCESS);
}

