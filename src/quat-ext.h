/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef QUAT_EXT_H
#define QUAT_EXT_H
#include <cglm/cglm.h>

void glm_quatd_from_euler(versord q, double z, double y, double x);
void glm_quatd_from_lon_lat(versord q, double lon, double lat);
void glm_quatd_from_ypr(versord q, double yaw, double pitch, double roll);
#endif /* QUAT_EXT_H */
