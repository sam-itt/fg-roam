/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdlib.h>
#include <string.h>

#include "stg-object.h"

StgObject *stg_object_init(StgObject *self, const char *filename)
{
    int len;
    char *last_slash;

    memset(self, 0, sizeof(StgObject));
    self->fp = fopen(filename, "r");
    if(!self->fp)
        return NULL;

    last_slash = strrchr(filename, '/');
    if(last_slash){
        /*last_slash+1 keeps the slash itself which well need later*/
        self->base_path = strndup(filename, (last_slash+1)-filename);
        self->bp_len = strlen(self->base_path);
    }
    return self;
}

StgObject *stg_object_dispose(StgObject *self)
{
    if(self->fp)
        fclose(self->fp);
    if(self->lbuf)
        free(self->lbuf);
    if(self->base_path)
        free(self->base_path);
    return self;
}

bool stg_object_get_value(StgObject *self, const char *verb, bool concat_base, char **out, size_t *n) {
    ssize_t read;
    size_t vlen;
    char *rv = NULL;

    // Validate input parameters.
    if (self == NULL || verb == NULL || out == NULL || n == NULL) {
        fprintf(stderr, "Debug: Invalid input parameters.\n");
        return false;
    }

    vlen = strlen(verb);
    printf("Debug: Starting stg_object_get_value with verb = '%s', concat_base = %d\n", verb, concat_base);

    while ((read = getline(&self->lbuf, &self->abuf, self->fp)) != -1) {
        printf("Debug: Read %zd bytes from file, line: '%s'\n", read, self->lbuf);

        if (!strncmp(self->lbuf, verb, vlen) && self->lbuf[vlen] == ' ') {
            printf("Debug: Found verb '%s' in line\n", verb);

            // Correct buffer size calculation to include the null terminator.
            size_t needed = read - (vlen + 1); // Excluding verb and the space, but including '\n'
            if (concat_base) {
                needed += self->bp_len; // Add base path length if needed.
            }

            printf("Debug: Needed buffer size: %zu, current base path length: %zu\n", needed, self->bp_len);

            // Ensure buffer is large enough.
            if (*out == NULL || needed > *n) {
                printf("Debug: Reallocating buffer. Current pointer: %p\n", (void *)*out);
                char *tmp = realloc(*out, needed * sizeof(char));
                if (!tmp) {
                    fprintf(stderr, "Debug: Realloc failed\n");
                    return false;
                }
                *out = tmp;
                *n = needed;
                printf("Debug: Reallocated buffer to %zu bytes. New pointer: %p\n", needed, (void *)*out);
            }

            // Concatenate base path if needed.
            if (concat_base) {
                strncpy(*out, self->base_path, self->bp_len);
                strncpy(*out + self->bp_len, self->lbuf + (vlen + 1), needed - self->bp_len);
                (*out)[needed - 1] = '\0'; // Ensure null-termination
                printf("Debug: Concatenated base path '%s' with data\n", self->base_path);
            } else {
                strncpy(*out, self->lbuf + (vlen + 1), needed);
                (*out)[needed - 1] = '\0'; // Ensure null-termination
            }

            printf("Debug: Set output to '%s', total length: %zu\n", *out, strlen(*out));
            return true;
        }
    }

    if (read == -1) {
        printf("Debug: Reached end of file or encountered an error.\n");
    }

    return false;
}
