#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include "texture.h"

static Texture *_store[256]; /*TODO: Array->Hash or embed in meshes*/
static unsigned char _ntextures = 0;


Texture *texture_new(const char *filename, const char *name)
{
    Texture *rv;

    rv = calloc(1, sizeof(Texture));
    if(rv){
        rv->filename = strdup(filename);
        if(name)
            rv->name = strdup(name);
    }
    if(!texture_load(rv)){
        texture_free(rv);
        return NULL;
    }
    _store[_ntextures++] = rv;
    return rv;
}

void texture_free(Texture *self)
{
    if(self->filename)
        free(self->filename);
    if(self->name)
        free(self->name);
    free(self);
}

bool texture_load(Texture *self)
{
    SDL_Surface *img;
    GLenum internal_format;
    GLenum format;

    img = IMG_Load(self->filename);
    if(!img){
        printf("SDL_Image couldn't load %s: %s\n",self->filename,SDL_GetError());
        return false;
    }

    glGenTextures(1, &(self->id));
    glBindTexture(GL_TEXTURE_2D, self->id);


    if(img->format->BytesPerPixel == 3){
        internal_format = GL_RGB;
        if(img->format->Rmask == 0xff)
            format = GL_RGB;
        else
            format = GL_BGR;
    }else if(img->format->BytesPerPixel == 4){    
        internal_format = GL_RGBA;
        if(img->format->Rmask == 0xff)
            format = GL_RGBA;
        else
            format = GL_BGRA;
    }else{
        printf("Unknown image format: %d Bytes per pixel\n",img->format->BytesPerPixel);
        SDL_FreeSurface(img);
        return false;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img->w, img->h, 0, format, GL_UNSIGNED_BYTE, img->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(img);
    return true;
}

Texture *texture_get_by_name(const char *name)
{
    for(int i = 0; i < _ntextures; i++){
        if(!strcmp(_store[i]->name,name))
            return _store[i];
    }
    return NULL;
}


GLuint texture_get_id_by_name(const char *name)
{
    Texture *t;

    t = texture_get_by_name(name);
/*    if(t)
        printf("%s is texture %d\n",name, t->id);    
    else
        printf("%s not found\n", name);*/
    return t ? t->id : 0;
}
