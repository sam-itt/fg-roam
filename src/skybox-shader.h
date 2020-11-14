#ifndef SKYBOX_SHADER_H
#define SKYBOX_SHADER_H

#include "shader.h"

typedef struct{
    Shader super;

    /*Attributes*/
    GLint position;

    /*Uniforms*/
    GLint projection_matrix;
    GLint view_matrix;
    GLint texunit;
}SkyboxShader;


SkyboxShader *skybox_shader_new(void);
SkyboxShader *skybox_shader_init(SkyboxShader *self);
void skybox_shader_dispose(SkyboxShader *self);
void skybox_shader_free(SkyboxShader *self);

#endif /* SKYBOX_SHADER_H */
