#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geo-location.h"

typedef struct{
    char *latitude;
    char *longitude;
}GeoLocationString;

GeoLocationString expected[] = {
    {
        .latitude =  "45.125555",
        .longitude = "5.717187"
    },
    {
        .latitude = "45.305419",
        .longitude = "5.972515"
    }
};

int main(int argc, char *argv[])
{
    double lat = 45.215487;
    double lon = 5.844851;
    double earthRadius = 6371.01;
    double distance = 10*1000; /*10 km*/
    char value[12];

    GeoLocation location, box[2];
    geo_location_set(&location, lat, lon);

    geo_location_bounding_coordinates(&location, distance, box);

    printf("Bounding box centered on lat:%f lon:%f is:\n"
        "up left - up right: (%f,%f) - (%f,%f)\n"
        "down left - down right: (%f,%f) - (%f,%f)\n",
        lat, lon,
        box[1].latitude, box[0].longitude,
        box[1].latitude, box[1].longitude,
        box[0].latitude, box[0].longitude,
        box[0].latitude, box[1].longitude
    );

    for(int i = 0; i < 2; i++){
        snprintf(value, 12, "%f",box[i].latitude);
        if(strcmp(expected[i].latitude, value)){
            printf("Error: %s != %s as it ought to be\n",
                expected[i].latitude,
                value
            );
            exit(EXIT_FAILURE);
        }

        snprintf(value, 12, "%f",box[i].longitude);
        if(strcmp(expected[i].longitude, value)){
            printf("Error: %s != %s as it ought to be\n",
                expected[i].longitude,
                value
            );
            exit(EXIT_FAILURE);
        }
    }

	exit(EXIT_SUCCESS);
}

