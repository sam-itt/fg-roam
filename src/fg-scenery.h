/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef FG_SCENERY_H
#define FG_SCENERY_H
#include <stdlib.h>

char *fg_scenery_get_file(const char *filename);
size_t fg_scenery_base_start(const char *filename);
#endif /* FG_SCENERY_H */
