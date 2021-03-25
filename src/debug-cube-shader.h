/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef DEBUG_CUBE_SHADER_H
#define DEBUG_CUBE_SHADER_H
#include "shader.h"

typedef struct{
    Shader super;

    /*Shader attributes*/
    GLint position;
    GLint color;
    /*Shader uniforms*/
    GLint mvp;
}DebugCubeShader;


DebugCubeShader *debug_cube_shader_new(void);
DebugCubeShader *debug_cube_shader_init(DebugCubeShader *self);
void debug_cube_shader_dispose(DebugCubeShader *self);
void debug_cube_shader_free(DebugCubeShader *self);


#endif /* DEBUG_CUBE_SHADER_H */
