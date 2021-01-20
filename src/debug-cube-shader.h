#ifndef DEBUG_CUBE_SHADER_H
#define DEBUG_CUBE_SHADER_H
#include "shader.h"

typedef struct{
    Shader super;

    /*Shader attributes*/
    GLint position;
    GLint color;
    /*Shader uniforms*/
    GLint mvp;
}DebugCubeShader;


DebugCubeShader *debug_cube_shader_new(void);
DebugCubeShader *debug_cube_shader_init(DebugCubeShader *self);
void debug_cube_shader_dispose(DebugCubeShader *self);
void debug_cube_shader_free(DebugCubeShader *self);


#endif /* DEBUG_CUBE_SHADER_H */
