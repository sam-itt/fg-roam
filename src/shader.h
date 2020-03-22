#ifndef SHADER_H
#define SHADER_H

#include <stdbool.h>
#include <GL/gl.h>

typedef struct{
    /*Source filenames*/
    char *vertex_src;
    char *fragment_src;

    GLuint vertex_id;
    GLuint fragment_id;
    GLuint program_id;
}Shader;


Shader *shader_new(const char *vertex, const char *fragment);
void shader_free(Shader *self);
bool shader_load(Shader *self);
void shader_bind_attribute(Shader *self, GLuint attribute, const char *name);
GLint shader_get_uniform_location(Shader *self, const char *name);
GLint shader_get_attribute_location(Shader *self, const char *name);



void shader_cleanup(Shader *self);
bool shader_compile_file(GLuint *shader, GLenum type, const char *filename);
#endif
