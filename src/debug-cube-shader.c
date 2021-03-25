/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>

#include "debug-cube-shader.h"

DebugCubeShader *debug_cube_shader_new(void)
{
    DebugCubeShader *rv;

    rv = calloc(1, sizeof(DebugCubeShader));
    if(rv){
        if(!debug_cube_shader_init(rv)){
            free(rv);
            return NULL;
        }
    }
    return rv;
}

DebugCubeShader *debug_cube_shader_init(DebugCubeShader *self)
{
    bool rv;

    if(!shader_init(SHADER(self), SHADER_DIR"/dc-vertex.gl", SHADER_DIR"/dc-fragment.gl"))
        return NULL;

    rv = shader_get_attribute_locationp(SHADER(self), "position", &(self->position));
    rv &= shader_get_attribute_locationp(SHADER(self), "color", &(self->color));
    rv &= shader_get_uniform_locationp(SHADER(self), "mvp", &(self->mvp));

    if(rv)
        return self;

    debug_cube_shader_dispose(self);
    return NULL;
}

void debug_cube_shader_dispose(DebugCubeShader *self)
{
    shader_dispose(SHADER(self));
}

void debug_cube_shader_free(DebugCubeShader *self)
{
    debug_cube_shader_dispose(self);
    free(self);
}

