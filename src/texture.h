/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef TEXTURE_H
#define TEXTURE_H
#include <stdbool.h>
#if USE_GLES
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

typedef struct{
    GLuint id;
    char *name;
    char *filename;
}Texture;

Texture *texture_new(const char *filename, const char *name);
void texture_free(Texture *self);

bool texture_load(Texture *self);
Texture *texture_get_by_name(const char *name);
GLuint texture_get_id_by_name(const char *name);


void texture_store_shutdown(void);
#endif
