#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include "shader.h"


static void shader_show_compile_error(GLuint shader, const char *shader_name);
static void shader_show_link_error(Shader *self);

Shader *shader_new(const char *vertex, const char *fragment)
{   
    Shader *rv;

    rv = calloc(1, sizeof(Shader));
    if(rv){
        rv->vertex_src = strdup(vertex);
        rv->fragment_src = strdup(fragment);
        if(!shader_load(rv)){
            shader_free(rv);
            rv = NULL;
        }
    }
    return rv;
}


void shader_free(Shader *self)
{
    shader_cleanup(self);
    if(self->vertex_src)
        free(self->vertex_src);
    if(self->fragment_src)
        free(self->fragment_src);
    free(self);
}

void shader_cleanup(Shader *self)
{
    if(glIsShader(self->vertex_id) == GL_TRUE)
        glDeleteShader(self->vertex_id);

    if(glIsShader(self->fragment_id) == GL_TRUE)
        glDeleteShader(self->fragment_id);

    if(glIsProgram(self->program_id) == GL_TRUE)
        glDeleteProgram(self->program_id);
}

bool shader_load(Shader *self)
{
    GLint rv;

    shader_cleanup(self);

    if(!shader_compile_file(&(self->vertex_id), GL_VERTEX_SHADER, self->vertex_src))
        return false;
    if(!shader_compile_file(&(self->fragment_id), GL_FRAGMENT_SHADER, self->fragment_src))
        return false;

    self->program_id = glCreateProgram();
    glAttachShader(self->program_id, self->vertex_id);
    glAttachShader(self->program_id, self->fragment_id);

    glLinkProgram(self->program_id);
    glGetProgramiv(self->program_id, GL_LINK_STATUS, &rv);
    if(rv != GL_TRUE){
        shader_show_link_error(self);
        glDeleteProgram(self->program_id);
        return false;
    }
    return true;
}

GLint shader_get_uniform_location(Shader *self, const char *name)
{
    return glGetUniformLocation(self->program_id, name);
}

GLint shader_get_attribute_location(Shader *self, const char *name)
{
    return glGetAttribLocation(self->program_id, name);
}


void shader_bind_attribute(Shader *self, GLuint attribute, const char *name)
{
    glBindAttribLocation(self->program_id, attribute, name);
}

bool shader_compile_file(GLuint *shader, GLenum type, const char *filename)
{
    FILE *fp;
    GLint fsize;
    char *content;
    GLint rv;

    *shader = glCreateShader(type);
    if(!(*shader)){
        printf("glCreateShader failed for type %d\n",type);
        return false;
    }

    fp = fopen(filename, "rb");
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);  /* same as rewind(fp); */

    content = malloc(sizeof(char)*fsize);
    fread(content, 1, fsize, fp);
    fclose(fp);

    glShaderSource(*shader, 1, (const GLchar * const*)&content, (const GLint *) &fsize);

    glCompileShader(*shader);
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &rv);
    free(content);

    if(rv != GL_TRUE){ /*Compile failed*/
        shader_show_compile_error(*shader, filename);
        glDeleteShader(*shader);
        return false;
    }
    return true;
}

static void shader_show_compile_error(GLuint shader, const char *shader_name)
{
    GLint msg_size;
    char *msg;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &msg_size);
    msg = calloc(msg_size + 1, sizeof(char));
    glGetShaderInfoLog(shader, msg_size, NULL, msg);

    printf("Shader %s compile error: %s\n",shader_name, msg);

    free(msg);
}

static void shader_show_link_error(Shader *self)
{
    GLint msg_size;
    char *msg;

    glGetProgramiv(self->program_id, GL_INFO_LOG_LENGTH, &msg_size);
    msg = calloc(msg_size + 1, sizeof(char));
    glGetShaderInfoLog(self->program_id, msg_size, NULL, msg);

    printf("Shader program %d compile error: %s\n", self->program_id, msg);

    free(msg);
}
