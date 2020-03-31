#ifndef MISC_H
#define MISC_H
#include <stdio.h>

#define KB_AMOUNT (1024.0)
#define MB_AMOUNT (1024*1024.0)
#define GB_AMOUNT (1024*1024*1024.0)
#define TB_AMOUNT (1024*1024*1024*1024.0)

#define KB_FACTOR (1024)
#define MB_FACTOR (1024*1024)
#define GB_FACTOR (1024*1024*1024)
#define TB_FACTOR (1024*1024*1024*1024)

#define kilobytes(bytes) (bytes*KB_FACTOR)
#define megabytes(bytes) (bytes*MB_FACTOR)
#define gigabytes(bytes) (bytes*GB_FACTOR)
#define terabytes(bytes) (bytes*TB_FACTOR)

#define MSEC_TO_SEC(msecs) ((msecs)/1000.0)



double normalize_periodicd(double min, double max, double value);

char *get_sized_unit_text(size_t amount);
double get_sized_unit_value(size_t amount);
#endif
