#ifndef SKYBOX_H
#define SKYBOX_H

#include <GL/gl.h>

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
