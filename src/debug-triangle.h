#ifndef DEBUG_TRIANGLE_H
#define DEBUG_TRIANGLE_H
#include <cglm/cglm.h>

#include "debug-triangle-shader.h"

typedef struct{
    DebugTriangleShader *shader;

    GLuint vbo;
    /*Matrices*/
    /*TODO: Look into using doubles*/
}DebugTriangle;

DebugTriangle *debug_triangle_new(void);
DebugTriangle *debug_triangle_init(DebugTriangle *self);

DebugTriangle *debug_triangle_dispose(DebugTriangle *self);
void *debug_triangle_free(DebugTriangle *self);

void debug_triangle_render(DebugTriangle *self);
#endif /* DEBUG_TRIANGLE_H */
