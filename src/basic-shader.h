#ifndef BASIC_SHADER_H
#define BASIC_SHADER_H

#include "shader.h"

typedef struct{
    Shader super;

    /*Shader attributes*/
    GLint position;
    GLint texcoords;
    /*Shader uniforms*/
    GLint mvp;
}BasicShader;


BasicShader *basic_shader_new(void);
BasicShader *basic_shader_init(BasicShader *self);
void basic_shader_dispose(BasicShader *self);
void basic_shader_free(BasicShader *self);

#endif /* BASIC_SHADER_H */
