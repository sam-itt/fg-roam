#define _GNU_SOURCE
//#define GL_VERSION_2_1
//#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cglm/cglm.h>
#if 0
#include <GL/gl.h>
#include <GL/glext.h>
#elif 0
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>


#include "skybox.h"

#define SIZE 1.0f

#ifndef SKY_ROOT
#define SKY_ROOT "."
#endif

static const char *faces[] = {
    SKY_ROOT"/resources/right.png",
    SKY_ROOT"/resources/left.png",
    SKY_ROOT"/resources/top.png",
    SKY_ROOT"/resources/bottom.png",
    SKY_ROOT"/resources/back.png",
    SKY_ROOT"/resources/front.png"
};
#if 1
static float vertices[] = {
	    -SIZE,  SIZE, -SIZE,
	    -SIZE, -SIZE, -SIZE,
	     SIZE, -SIZE, -SIZE,
	     SIZE, -SIZE, -SIZE,
	     SIZE,  SIZE, -SIZE,
	    -SIZE,  SIZE, -SIZE,

	    -SIZE, -SIZE,  SIZE,
	    -SIZE, -SIZE, -SIZE,
	    -SIZE,  SIZE, -SIZE,
	    -SIZE,  SIZE, -SIZE,
	    -SIZE,  SIZE,  SIZE,
	    -SIZE, -SIZE,  SIZE,

	     SIZE, -SIZE, -SIZE,
	     SIZE, -SIZE,  SIZE,
	     SIZE,  SIZE,  SIZE,
	     SIZE,  SIZE,  SIZE,
	     SIZE,  SIZE, -SIZE,
	     SIZE, -SIZE, -SIZE,

	    -SIZE, -SIZE,  SIZE,
	    -SIZE,  SIZE,  SIZE,
	     SIZE,  SIZE,  SIZE,
	     SIZE,  SIZE,  SIZE,
	     SIZE, -SIZE,  SIZE,
	    -SIZE, -SIZE,  SIZE,

	    -SIZE,  SIZE, -SIZE,
	     SIZE,  SIZE, -SIZE,
	     SIZE,  SIZE,  SIZE,
	     SIZE,  SIZE,  SIZE,
	    -SIZE,  SIZE,  SIZE,
	    -SIZE,  SIZE, -SIZE,

	    -SIZE, -SIZE, -SIZE,
	    -SIZE, -SIZE,  SIZE,
	     SIZE, -SIZE, -SIZE,
	     SIZE, -SIZE, -SIZE,
	    -SIZE, -SIZE,  SIZE,
	     SIZE, -SIZE,  SIZE
};
#else
static float vertices[24] = {
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    -1.0,  1.0,  1.0,
    1.0,  1.0,  1.0,
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
    1.0,  1.0, -1.0,
};
static GLubyte indices[14] = {0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1};


#endif

bool skybox_load_textures(Skybox *self);
void skybox_prepare_vbo(Skybox *self);

Skybox *skybox_new(mat4d projection)
{
    Skybox *self;

    self = calloc(1, sizeof(Skybox));
    if(self){
        skybox_init(self, projection);
    }
    return self;

}

Skybox *skybox_init(Skybox *self, mat4d projection)
{
    skybox_load_textures(self);
    skybox_prepare_vbo(self);

    self->shader = skybox_shader_new();

    skybox_set_projection(self, projection);

    glm_mat4_identity(self->view);
    /*Sets the uniform once and for all*/
#if 0
    glUseProgram(SHADER(self->shader)->program_id);
    glUniform1i(self->shader->texunit, 0);
    glUseProgram(0);
#endif

    return self;
}

Skybox *skybox_dispose(Skybox *self)
{
    glDeleteTextures(1, &self->tex_id);
    glDeleteBuffers(1, &self->vertex_buffer);
#if 0 /*Currently unused*/
    glDeleteBuffers(1, &indices_buffer);
#endif
    skybox_shader_free(self->shader);

    return self;
}

Skybox *skybox_free(Skybox *self)
{
    skybox_dispose(self);
    free(self);
    return NULL;
}

void skybox_set_projection(Skybox *self, mat4d projection)
{
    mat4 projf;
    glm_mat4d_ucopyf(projection, projf);

    glUseProgram(SHADER(self->shader)->program_id);
    glUniformMatrix4fv(self->shader->projection_matrix, 1, GL_FALSE, projf[0]);
    glUseProgram(0);
}

void skybox_render(Skybox *self)
{
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_CUBE_MAP);
    glUseProgram(SHADER(self->shader)->program_id);

    glUniformMatrix4fv(self->shader->view_matrix, 1, GL_FALSE, self->view[0]);
    glUniform1i(self->shader->texunit, 0);


    glBindTexture(GL_TEXTURE_CUBE_MAP, self->tex_id);

    glEnableVertexAttribArray(self->shader->position);
    glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0, /*no need to specify stride on single attribute vectors*/
        (void*)0
    );
#if 0
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indices_buffer);
    glDrawElements(GL_TRIANGLE_STRIP,
        14,
        GL_UNSIGNED_BYTE, 0
    );
#else
    glDrawArrays(GL_TRIANGLES, 0, 6*6);
#endif
//    glDisableVertexAttribArray(self->pos_attr);
    glUseProgram(0);
    glDepthFunc(GL_LESS);
}

bool skybox_load_textures(Skybox *self)
{
    SDL_Surface *img;
    GLenum format;
    GLenum internal_format;

    glGenTextures(1, &(self->tex_id));
    glBindTexture(GL_TEXTURE_CUBE_MAP, self->tex_id);

    for(int i = 0; i < 6; i++){
        img = IMG_Load(faces[i]);
        if(!img){
            printf("SDL_Image couldn't load %s: %s\n",faces[i],SDL_GetError());
            return false;
        }

        if(img->format->BytesPerPixel == 3){
            internal_format = GL_RGB;
            if(img->format->Rmask == 0xff)
                format = GL_RGB;
            else
#if 0
                format = GL_BGR;
#else
                goto bail;
#endif
        }else if(img->format->BytesPerPixel == 4){
            internal_format = GL_RGBA;
            if(img->format->Rmask == 0xff)
                format = GL_RGBA;
            else
#if 0
                format = GL_BGRA;
#else
                goto bail;
#endif
        }else{
bail:
            printf("Error while loading %s: Unknown image format, %d Bytes per pixel\n",faces[i],img->format->BytesPerPixel);
            exit(0);
            SDL_FreeSurface(img);
            return false;
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     internal_format,
                     img->w, img->h,
                     0, format,
                     GL_UNSIGNED_BYTE,
                     img->pixels
        );
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


        SDL_FreeSurface(img);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return true;
}

void skybox_prepare_vbo(Skybox *self)
{

    glGenBuffers(1, &(self->vertex_buffer));
    glBindBuffer(GL_ARRAY_BUFFER, self->vertex_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );
#if 0
    glGenBuffers(1, &(self->indices_buffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->indices_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(indices),
        indices,
        GL_STATIC_DRAW
    );
#endif
//    printf("sizeof(vertices): %d, sizeof(vertices)(sizeof(float) = %d\n",sizeof(vertices), sizeof(vertices)/sizeof(float));
}
