#ifndef TEXTURE_H
#define TEXTURE_H
#include <stdbool.h>
#if 0
#include <GL/gl.h>
#else
#include <GLES2/gl2.h>
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
