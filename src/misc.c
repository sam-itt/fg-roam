/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <libgen.h>

#include "misc.h"

// normalize the value to be in a range between [min, max[
double normalize_periodicd(double min, double max, double value)
{
    double range = max - min;
    if (range < DBL_MIN)
      return min;
    double normalized = value - range*floor((value - min)/range);
    // two security checks that can only happen due to roundoff
    if (normalized <= min)
      return min;
    if (max <= normalized)
      return min;
    return normalized;
}

char *get_sized_unit_text(size_t amount)
{
	if(amount >= GB_AMOUNT )
		return("GB");
	if(amount >= MB_AMOUNT )
		return("MB");
	if(amount >= KB_AMOUNT )
		return("KB");
	return("Bytes");
}

double get_sized_unit_value(size_t amount)
{
	if(amount >= GB_AMOUNT ){
		return(amount/GB_AMOUNT);
	}
	if(amount >= MB_AMOUNT ){
		return(amount/MB_AMOUNT);
	}
	if(amount >= KB_AMOUNT ){
		return(amount/KB_AMOUNT);
	}
	return(amount);
}

#ifndef HAVE_MKDIR_P
/**
 * @brief Creates directory @param dir, creating parents as needed.
 *
 * Same behavior as the shell command mkdir -p
 *
 *
 * @param dir The directory (path/hierarchy) to create
 * @param mode The creation mode, bitfield of permissions.
 * Refer to stat(2).
 *
 * credits: http://nion.modprobe.de/blog/archives/357-Recursive-directory-creation.html
 */
void mkdir_p(const char *dir, mode_t mode)
{
    char tmp[PATH_MAX];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++){
        if(*p == '/') {
            *p = 0;
            mkdir(tmp, mode);
            *p = '/';
        }
    }
    mkdir(tmp, mode);
}
#endif

#ifndef HAVE_CREATE_PATH
/**
 * @brief Create all directories leading to @param filename
 *
 * @param filename Path to a file.
 */
bool create_path(const char *filename)
{
    char *tmp;
    char *dname;
    size_t len;

    /*dirname can modify its argument, so we need to make a copy*/
    len = strlen(filename);
    if(len < 1024)
        tmp = strdupa(filename);
    else
        tmp = strdup(filename);
    dname = dirname(tmp);

    if(access(dname, F_OK) != 0)
        mkdir_p(dname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(len >= 1024) free(tmp);

    return access(dname, F_OK) == 0;
}
#endif

#ifdef ENABLE_TEST
int main(int argc, char *argv[])
{


	exit(EXIT_SUCCESS);
}
#endif

