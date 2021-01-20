#ifndef DEBUG_TRIANGLE_SHADER_H
#define DEBUG_TRIANGLE_SHADER_H
#include "shader.h"

typedef struct{
    Shader super;

    /*Shader attributes*/
    GLint position;
    GLint color;
    /*Shader uniforms*/
    GLint mvp;
}DebugTriangleShader;


DebugTriangleShader *debug_triangle_shader_new(void);
DebugTriangleShader *debug_triangle_shader_init(DebugTriangleShader *self);
void debug_triangle_shader_dispose(DebugTriangleShader *self);
void debug_triangle_shader_free(DebugTriangleShader *self);


#endif /* DEBUG_TRIANGLE_SHADER_H */
