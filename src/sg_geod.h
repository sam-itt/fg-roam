/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef SG_GEOD_H
#define SG_GEOD_H

void SGGeodToCart(double latitude, double longitude, double altitude, double *X, double *Y, double *Z);

#endif /* SG_GEOD_H */
