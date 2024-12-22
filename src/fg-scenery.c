/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include "bucket.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "misc.h"
#include "http-download.h"
#include "fgr-dirs.h"

#ifndef FG_MIRROR_URL
#define FG_MIRROR_URL "https://flightgear.sourceforge.net/scenery/Terrain"
#endif


char *fg_scenery_get_file(const char *filename)
{
    char *rv;
    size_t flen;
    bool is_btg;

    /*.btg files are referenced as .btg but also can  - and most of the time
     * are - in fact .btg.gz. This adds supporting code and can lead to
     * failures.
     *
     * The SGBinObject "translated" from Flightgear always tries .btg first and
     * then .btg.gz.
     *
     * The mirror seems to only have .btg.gz files, so here we assume that .btg
     * means in fact .btg.gz. and completly ignore a .btg-only code path.
     *
     * It might be better to assume that it's always .btz.gz and keep the
     * alternative codepath as a fallback and/or run a script to ensure it's one
     * way or another and de-clutter the main C code from this alternative.
     * TODO: Sort this out.
     * */
    flen = strlen(filename);
    is_btg =   filename[flen-4] == '.'
            && filename[flen-3] == 'b'
            && filename[flen-2] == 't'
            && filename[flen-1] == 'g';
    if(is_btg)
        asprintf(&rv, TERRAIN_DIR"/%s.gz", filename);
    else
        asprintf(&rv, TERRAIN_DIR"/%s", filename);
    if(access(rv, F_OK) != 0){
        /*  This is downloading feature is not intended to make it
         *  into the final version. Terrain/Airports/etc deployed/installed
         *  as a whole (maybe using a grabbing script) and not one by one
         *  and certainly not at runtime.
         *
         *  This feature is nonetheless very useful for the dev version
         *  and for demos.
         * */
        char *url; /*TODO: alloca*/
        if(is_btg)
            asprintf(&url, "%s/%s.gz", FG_MIRROR_URL, filename);
        else
            asprintf(&url, "%s/%s", FG_MIRROR_URL, filename);
        if(!http_download_file(url, rv)){
            printf("Failure to download %s\n", url);
            exit(0);
            free(rv);
            rv = NULL;
        }
        free(url);
    }
    return rv;
}

size_t fg_scenery_base_start(const char *filename)
{
    if(strstr(filename, TERRAIN_DIR))
        return strlen(TERRAIN_DIR);
    return 0;

}
