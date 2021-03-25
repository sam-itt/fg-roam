/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef SHADER_H
#define SHADER_H
#include <stdbool.h>

#if USE_GLES
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#include "fgr-dirs.h"

typedef struct{
    /*Source filenames*/
    char *vertex_src;
    char *fragment_src;

    GLuint vertex_id;
    GLuint fragment_id;
    GLuint program_id;
}Shader;

#define SHADER(self) ((Shader*)(self))

Shader *shader_new(const char *vertex, const char *fragment);
Shader *shader_init(Shader *self, const char *vertex, const char *fragment);
void *shader_dispose(Shader *self);
void shader_free(Shader *self);

void shader_bind_attribute(Shader *self, GLuint attribute, const char *name);
GLint shader_get_uniform_location(Shader *self, const char *name);
GLint shader_get_attribute_location(Shader *self, const char *name);

bool shader_get_uniform_locationp(Shader *self, const char *name, GLint *uniform);
bool shader_get_attribute_locationp(Shader *self, const char *name, GLint *attribute);

#endif
