/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef STG_OBJECT_H
#define STG_OBJECT_H
#include <stdio.h>
#include <stdbool.h>

typedef struct{
    FILE *fp;

    char *base_path;
    size_t bp_len;
    /*first call to getline will set us*/
    char *lbuf; /*line reading buffer*/
    size_t abuf; /*allocated size*/
}StgObject;

StgObject *stg_object_init(StgObject *self, const char *filename);
StgObject *stg_object_dispose(StgObject *self);

bool stg_object_get_value(StgObject *self, const char *verb,
                          bool concat_base, char **out, size_t *n);

#endif /* STG_OBJECT_H */
