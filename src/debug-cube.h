#ifndef DEBUG_CUBE_H
#define DEBUG_CUBE_H
#include <cglm/cglm.h>

#include "debug-cube-shader.h"

typedef struct{
    DebugCubeShader *shader;

    GLuint vbo;
    GLuint colors;
    /*Matrices*/
    mat4 projection;
    mat4 view;
    mat4 mvp;
    /*TODO: Look into using doubles*/
}DebugCube;

DebugCube *debug_cube_new(void);
DebugCube *debug_cube_init(DebugCube *self);

DebugCube *debug_cube_dispose(DebugCube *self);
void *debug_cube_free(DebugCube *self);

void debug_cube_render(DebugCube *self);
#endif /* DEBUG_CUBE_H */
