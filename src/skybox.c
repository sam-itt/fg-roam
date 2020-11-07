#include "cglm/mat4d.h"
#define _GNU_SOURCE
#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cglm/cglm.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include "skybox.h"

#define SIZE 1.0f

static const char *faces[] = {
    "resources/right.png",
    "resources/left.png",
    "resources/top.png",
    "resources/bottom.png",
    "resources/back.png",
    "resources/front.png"
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
    GLuint mtx;

    skybox_load_textures(self);
    skybox_prepare_vbo(self);

    self->shader = shader_new("sky-vertex.gl", "sky-fragment.gl");
    mtx = shader_get_uniform_location(self->shader, "projectionMatrix");
    if(mtx < 0){
        printf("Cannot bind skybox projection matrix\n");
        exit(-1);
    }
    self->mtx_u = mtx;

    self->vmtx_u = shader_get_uniform_location(self->shader, "viewMatrix");
    if(self->vmtx_u < 0){
        printf("Cannot bind skybox view matrix\n");
//        exit(-1);
    }

    self->pos_attr = shader_get_attribute_location(self->shader, "position");
    if(self->pos_attr < 0){
        printf("Cannot bind skybox position\n");
        exit(-1);
    }

    self->texunit = shader_get_uniform_location(self->shader, "cubeMap");
    if(self->texunit < 0){
        printf("Cannot bind skybox texunit\n");
        exit(-1);
    }
    mat4 projf;
    glm_mat4d_ucopyf(projection, projf);

    glUseProgram(self->shader->program_id);
    glUniformMatrix4fv(mtx, 1, GL_FALSE, projf[0]);
    glUniform1i(self->texunit, 0);
    glUseProgram(0);


    return self;
}

void skybox_render(Skybox *self, mat4d view)
{
    mat4 fview;

    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_CUBE_MAP);
    glUseProgram(self->shader->program_id);

    glm_mat4d_ucopyf(view, fview);
/*    fview[3][0] = 0.0;
    fview[3][1] = 0.0;
    fview[3][2] = 0.0;*/
//    glm_scale_to(view, (vec3){2.0,2.0,1.0}, fview);

    glUniformMatrix4fv(self->vmtx_u, 1, GL_FALSE, fview[0]);
    glUniform1i(self->texunit, 0);


    glBindTexture(GL_TEXTURE_CUBE_MAP, self->tex_id);

    glEnableVertexAttribArray(self->pos_attr);
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
                format = GL_BGR;
        }else if(img->format->BytesPerPixel == 4){
            internal_format = GL_RGBA;
            if(img->format->Rmask == 0xff)
                format = GL_RGBA;
            else
                format = GL_BGRA;
        }else{
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
    printf("sizeof(vertices): %d, sizeof(vertices)(sizeof(float) = %d\n",sizeof(vertices), sizeof(vertices)/sizeof(float));
}
