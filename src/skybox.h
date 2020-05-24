#ifndef SKYBOX_H
#define SKYBOX_H

#include <GL/gl.h>

#include "shader.h"
#include "cglm/types.h"

typedef struct{
    GLuint tex_id;
    GLuint vertex_buffer;
    GLuint indices_buffer;

    Shader *shader;
    GLint pos_attr;
    GLint mtx_u;
    GLint vmtx_u;
    GLint texunit;

}Skybox;

//uGLuint skybox_load_textures(void);

Skybox *skybox_new(mat4 projection);
Skybox *skybox_init(Skybox *self, mat4 projection);

void skybox_render(Skybox *self, mat4 view);
#endif /* SKYBOX_H */
