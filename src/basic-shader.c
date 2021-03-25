/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>

#include "basic-shader.h"

BasicShader *basic_shader_new(void)
{
    BasicShader *rv;

    rv = calloc(1, sizeof(BasicShader));
    if(rv){
        if(!basic_shader_init(rv)){
            free(rv);
            return NULL;
        }
    }
    return rv;
}

BasicShader *basic_shader_init(BasicShader *self)
{
    bool rv;

    if(!shader_init(SHADER(self), SHADER_DIR"/vertex.gl", SHADER_DIR"/fragment.gl"))
        return NULL;

    rv = shader_get_attribute_locationp(SHADER(self), "position", &(self->position));
    rv &= shader_get_attribute_locationp(SHADER(self), "texcoord", &(self->texcoords));
    rv &= shader_get_uniform_locationp(SHADER(self), "mvp", &(self->mvp));

    if(rv)
        return self;

    basic_shader_dispose(self);
    return NULL;
}

void basic_shader_dispose(BasicShader *self)
{
    shader_dispose(SHADER(self));
}

void basic_shader_free(BasicShader *self)
{
    basic_shader_dispose(self);
    free(self);
}
