/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef SKYBOX_H
#define SKYBOX_H
#if USE_GLES
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#include "skybox-shader.h"
#include "cglm/types.h"

typedef struct{
    GLuint tex_id;
    GLuint vertex_buffer;
    GLuint indices_buffer;

    SkyboxShader *shader;

    mat4 view;
}Skybox;

//uGLuint skybox_load_textures(void);

Skybox *skybox_new(mat4d projection);
Skybox *skybox_init(Skybox *self, mat4d projection);

Skybox *skybox_dispose(Skybox *self);
Skybox *skybox_free(Skybox *self);

void skybox_set_projection(Skybox *self, mat4d projection);
void skybox_render(Skybox *self);
#endif /* SKYBOX_H */
