/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <math.h>

#include "quat-ext.h"

/*Angles in Radians*/
void glm_quatd_from_euler(versord q, double z, double y, double x)
{
    double zd2 = (double)(0.5)*z; double yd2 = (double)(0.5)*y; double xd2 = (double)(0.5)*x;
    double Szd2 = sin(zd2); double Syd2 = sin(yd2); double Sxd2 = sin(xd2);
    double Czd2 = cos(zd2); double Cyd2 = cos(yd2); double Cxd2 = cos(xd2);
    double Cxd2Czd2 = Cxd2*Czd2; double Cxd2Szd2 = Cxd2*Szd2;
    double Sxd2Szd2 = Sxd2*Szd2; double Sxd2Czd2 = Sxd2*Czd2;

    glm_quatd_init(q,
        Sxd2Czd2*Cyd2 - Cxd2Szd2*Syd2, //x
        Cxd2Czd2*Syd2 + Sxd2Szd2*Cyd2, //y
        Cxd2Szd2*Cyd2 - Sxd2Czd2*Syd2, //z
        Cxd2Czd2*Cyd2 + Sxd2Szd2*Syd2 //w
    );
}

/*lon and lat in degrees*/
void glm_quatd_from_lon_lat(versord q, double lon, double lat)
{
    /*Convert to Radians*/
    lon *= (M_PI/180.0);
    lat *= (M_PI/180.0);

    double zd2 = (double)(0.5)*lon;
    double yd2 = (double)(-0.25)*M_PI - (double)(0.5)*lat;
    double Szd2 = sin(zd2);
    double Syd2 = sin(yd2);
    double Czd2 = cos(zd2);
    double Cyd2 = cos(yd2);

    glm_quatd_init(q,
        -Szd2*Syd2, //x
        Czd2*Syd2, //y
        Szd2*Cyd2, //z
        Czd2*Cyd2 //w
    );
}

/*Angles in degrees*/
void glm_quatd_from_ypr(versord q, double yaw, double pitch, double roll)
{
    glm_quatd_from_euler(q,
        yaw * (M_PI/180.0),
        pitch * (M_PI/180.0),
        roll * (M_PI/180.0)
    );
}
