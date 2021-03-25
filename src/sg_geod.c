/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define _SQUASH    0.9966471893352525192801545
#define _EQURAD 6378137.0
#define E2 fabs(1 - _SQUASH*_SQUASH)


/*Degrees / meters*/
void SGGeodToCart(double latitude, double longitude, double altitude, double *X, double *Y, double *Z)
{
  // according to
  // H. Vermeille,
  // Direct transformation from geocentric to geodetic ccordinates,
  // Journal of Geodesy (2002) 76:451-454
  double lambda = longitude * M_PI/180.0;
  double phi = latitude * M_PI/180.0;
  double h = altitude;
  double sphi = sin(phi);
  double n = _EQURAD/sqrt(1-E2*sphi*sphi);
  double cphi = cos(phi);
  double slambda = sin(lambda);
  double clambda = cos(lambda);
  *X = (h+n)*cphi*clambda;
  *Y = (h+n)*cphi*slambda;
  *Z = (h+n-E2*n)*sphi;
}


