/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>

#include "debug-triangle-shader.h"

DebugTriangleShader *debug_triangle_shader_new(void)
{
    DebugTriangleShader *rv;

    rv = calloc(1, sizeof(DebugTriangleShader));
    if(rv){
        if(!debug_triangle_shader_init(rv)){
            free(rv);
            return NULL;
        }
    }
    return rv;
}

DebugTriangleShader *debug_triangle_shader_init(DebugTriangleShader *self)
{
    bool rv;

    if(!shader_init(SHADER(self), SHADER_DIR"/dt-vertex.gl", SHADER_DIR"/dt-fragment.gl"))
        return NULL;

    rv = shader_get_attribute_locationp(SHADER(self), "position", &(self->position));
//    rv &= shader_get_attribute_locationp(SHADER(self), "color", &(self->color));
    rv &= shader_get_uniform_locationp(SHADER(self), "mvp", &(self->mvp));

    if(rv)
        return self;

    debug_triangle_shader_dispose(self);
    return NULL;
}

void debug_triangle_shader_dispose(DebugTriangleShader *self)
{
    shader_dispose(SHADER(self));
}

void debug_triangle_shader_free(DebugTriangleShader *self)
{
    debug_triangle_shader_dispose(self);
    free(self);
}

