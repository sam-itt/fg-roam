/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "frustum-ext.h"
#include <math.h>

/*TODO: Have sphered into cglm exts*/
bool glm_frustum_cgsphered(vec4 frustum[6], SGSphered *sphere)
{
    vec3 normal;
    vec3 spos; /*sphere position*/
    float dist;
    float side;

    spos[0] = sphere->center.x;
    spos[1] = sphere->center.y;
    spos[2] = sphere->center.z;

    for(int i = 0; i < 6; i++){
        normal[0] = frustum[i][0];
        normal[1] = frustum[i][1];
        normal[2] = frustum[i][2];
        dist = frustum[i][3];
        side = glm_vec3_dot(spos, normal) + dist;
        if(side < -sphere->radius)
            return false;
    }
    return true;
}

/**
 * Creates a bounding sphere around frustum
 *
 *
 */
//Exracted planes order: [left, right, bottom, top, near, far]
void glm_frustum_bounding_sphere(vec4 frustum[6], vec3 look_vector, vec3 cam_pos, float near, float fov_rad, vec4 sphere)
{
    float view_len;
    float height, width;
    vec3 p,q,vdiff;

    view_len = frustum[5][3] - frustum[4][3];

    height = view_len * tan(fov_rad * 0.5f);
    width = height;

    p[0] = 0.0f;
    p[1] = 0.0f;
    p[2] = frustum[4][3] + view_len * 0.5f;

    q[0] = width;
    q[1] = height;
    q[2] = view_len;

    glm_vec3_sub(p, q, vdiff);
    sphere[3] = glm_vec3_norm(vdiff);

    vec3 center;
    vec3 tmp;
    glm_vec3_scale(look_vector, view_len*0.5f, tmp);
    glm_vec3_adds(tmp, near, tmp);
    glm_vec3_add(cam_pos, tmp, center);
    sphere[0] = center[0];
    sphere[1] = center[1];
    sphere[2] = center[2];
}
