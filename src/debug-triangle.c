#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>

#if USE_GLES
#include <SDL2/SDL_opengles2.h>
#include <SDL_opengles2_gl2ext.h>
#else
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#endif


#include "debug-triangle-shader.h"
#include "debug-triangle.h"

// An array of 3 vectors which represents 3 vertices
static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   0.0f,  1.0f, 0.0f,
};

DebugTriangle *debug_triangle_new(void)
{
    DebugTriangle *self;

    self = calloc(1, sizeof(DebugTriangle));
    if(self){
        if(!debug_triangle_init(self))
            return debug_triangle_free(self);
    }
    return self;
}

DebugTriangle *debug_triangle_init(DebugTriangle *self)
{
    self->shader = debug_triangle_shader_new();

    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    return self;
}

DebugTriangle *debug_triangle_dispose(DebugTriangle *self)
{
    return self;
}

void *debug_triangle_free(DebugTriangle *self)
{
    free(debug_triangle_dispose(self));
    return NULL;
}

void debug_triangle_render(DebugTriangle *self)
{
    glUseProgram(SHADER(self->shader)->program_id);

    glUniformMatrix4fv(self->shader->mvp, 1, GL_FALSE, GLM_MAT4_IDENTITY[0]);


    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(self->shader->position);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glVertexAttribPointer(
       self->vbo,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       0,                  // stride
       (void*)0            // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    glDisableVertexAttribArray(self->vbo);

    glUseProgram(0);
}
