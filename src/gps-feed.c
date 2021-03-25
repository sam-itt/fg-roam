/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>

#include "gps-feed.h"

void gps_record_dump(GpsRecord *self)
{
    printf("GpsRecord %p: time: %ld, lat: %0.17g, lon: %0.17g, alt: %0.17g\n",self,self->time,self->lat,self->lon,self->alt);
}

