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

#include "debug-cube-shader.h"
#include "debug-cube.h"

// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f,-1.0f,-1.0f, // triangle 1 : begin
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, // triangle 1 : end
    1.0f, 1.0f,-1.0f, // triangle 2 : begin
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f, // triangle 2 : end
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

// One color for each vertex. They were generated randomly.
static const GLfloat g_color_buffer_data[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
};

DebugCube *debug_cube_new(void)
{
    DebugCube *self;

    self = calloc(1, sizeof(DebugCube));
    if(self){
        if(!debug_cube_init(self))
            return debug_cube_free(self);
    }
    return self;
}

DebugCube *debug_cube_init(DebugCube *self)
{
    self->shader = debug_cube_shader_new();

    glm_perspective(glm_rad(45.0f), 800.0/600.0, 0.1f, 100.0f, self->projection);
    glm_lookat((vec3){4,3,3}, (vec3){0,0,0}, (vec3){0,1,0}, self->view);
    glm_mat4_mul(self->projection, self->view, self->mvp);

    glGenBuffers(1, &self->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &self->colors);
    glBindBuffer(GL_ARRAY_BUFFER, self->colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    return self;
}

DebugCube *debug_cube_dispose(DebugCube *self)
{
    return self;
}

void *debug_cube_free(DebugCube *self)
{
    free(debug_cube_dispose(self));
    return NULL;
}

void debug_cube_render(DebugCube *self)
{

    glDisable(GL_DEPTH_TEST);   // skybox should be drawn behind anything else
    glUseProgram(SHADER(self->shader)->program_id);

    glUniformMatrix4fv(self->shader->mvp, 1, GL_FALSE, self->mvp[0]);


    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(self->shader->position);
    glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
    glVertexAttribPointer(
       self->shader->position,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
       3,                  // size
       GL_FLOAT,           // type
       GL_FALSE,           // normalized?
       3*sizeof(float),                  // stride
       (void*)0            // array buffer offset
    );

    glEnableVertexAttribArray(self->shader->color);
    glBindBuffer(GL_ARRAY_BUFFER, self->colors);
    glVertexAttribPointer(
        self->shader->color,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        3*sizeof(float),                                // stride
        (void*)0                          // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares
    glDisableVertexAttribArray(self->vbo);
    glDisableVertexAttribArray(self->colors);

    glUseProgram(0);
}
