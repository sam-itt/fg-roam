//#define GL_VERSION_2_1
//#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if 0
#include <GL/gl.h>
#include <GL/glext.h>
#elif 0
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>


#include "shader.h"

/*Private functions*/
static void shader_cleanup(Shader *self);
static bool shader_compile_file(GLuint *shader, GLenum type, const char *filename);
static void shader_show_compile_error(GLuint shader, const char *shader_name);
static void shader_show_link_error(Shader *self);
static bool shader_load(Shader *self);

/**
 * @brief Creates a new shader from the given files.
 *
 * This will trigger compile on the GPU and fail if the program can't
 * be put together.
 *
 * Calling code must call shader_free() when done.
 *
 * @param vertex Path to the vertex shader.
 * @param fragment Path to the fragment shader.
 * @return a usable shader or NULL on failure.
 *
 * @see shader_free
 */
Shader *shader_new(const char *vertex, const char *fragment)
{
    Shader *rv;

    rv = calloc(1, sizeof(Shader));
    if(rv){
        if(!shader_init(rv, vertex, fragment)){
            free(rv);
            return NULL;
        }
    }
    return rv;
}

/**
 * @brief Inits a shader with given files.
 *
 * Behaves just like shader_new except it doesn't allocate
 * memory for @p self and must be given that location.
 *
 * Use this function when creating derived types of Shader or
 * using a statically allocated Shader.
 *
 * Calling code must call shader_dispose() when done.
 *
 * @param vertex Path to the vertex shader.
 * @param fragment Path to the fragment shader.
 * @return a usable shader or NULL on failure.
 *
 * @see shader_dispose
 * @see shader_new
 */
Shader *shader_init(Shader *self, const char *vertex, const char *fragment)
{
    self->vertex_src = strdup(vertex);
    self->fragment_src = strdup(fragment);
    if(!self->vertex_src || !self->fragment_src){
        return shader_dispose(self);
    }

    if(!shader_load(self)){
        return shader_dispose(self);
    }
    return self;
}

/**
 * @brief Release any resource held by @p self.
 *
 * Does NOT free self itself, only the resources it
 * may hold.
 *
 * @param self a Shader
 * @return Always NULL (convenience feature)
 */
void *shader_dispose(Shader *self)
{
    shader_cleanup(self);
    if(self->vertex_src)
        free(self->vertex_src);
    if(self->fragment_src)
        free(self->fragment_src);
    return NULL;
}

/**
 * @brief Do over with @p self.
 *
 * Also free any resources internaly held by it
 *
 * @param self a Shader
 */
void shader_free(Shader *self)
{
    shader_dispose(self);
    free(self);
}

/*TODO: Inline or macro*/
/**
 * @brief Returns the OpenGL handle for a shader uniform.
 *
 * @param self a Shader
 * @param name the name of the uniform to look for
 * @returns OpenGL handle (>0) on success, a value <0 otherwise.
 */
GLint shader_get_uniform_location(Shader *self, const char *name)
{
    return glGetUniformLocation(self->program_id, name);
}

/*TODO: Inline or macro*/
/**
 * @brief Returns the OpenGL handle for a shader attribute.
 *
 * @param self a Shader
 * @param name the name of the attribute to look for
 * @returns OpenGL handle (>0) on success, a value <0 otherwise.
 */
GLint shader_get_attribute_location(Shader *self, const char *name)
{
    return glGetAttribLocation(self->program_id, name);
}

/**
 * @brief Fills @p uniform with the OpenGL handle for a shader uniform.
 *
 * Works like shader_get_uniform_location but fills in the value instead
 * of returning it. This function will always modify uniform setting it
 * either to a working handle or a negative value.
 *
 * @param self a Shader
 * @param name the name of the uniform to look for
 * @param uniform Where to write the result.
 * @returns true on success, false otherwise.
 */
bool shader_get_uniform_locationp(Shader *self, const char *name, GLint *uniform)
{
    *uniform = glGetUniformLocation(self->program_id, name);
    if(*uniform < 0){
        printf("Failed to lookup uniform %s in shader %s + %s\n",
            name,
            self->vertex_src,
            self->fragment_src
        );
        return false;
    }
    return *uniform >= 0;
}

/**
 * @brief Fills @p attribute with the OpenGL handle for a shader attribute.
 *
 * Works like shader_get_attribute_location but fills in the value instead
 * of returning it. This function will always modify attribute setting it
 * either to a working handle or a negative value.
 *
 * @param self a Shader
 * @param name the name of the attribute to look for
 * @param attribute Where to write the result.
 * @returns true on success, false otherwise.
 */
bool shader_get_attribute_locationp(Shader *self, const char *name, GLint *attribute)
{
    *attribute = glGetAttribLocation(self->program_id, name);
    if(*attribute < 0){
        printf("Failed to lookup attribute %s in shader %s + %s\n",
            name,
            self->vertex_src,
            self->fragment_src
        );
        return false;
    }
    return *attribute >= 0;
}

/**
 * @brief Wrapper around glBindAttribLocation.
 *
 * HAS CURRENTLY NO USE CASE. REMOVE?
 */
void shader_bind_attribute(Shader *self, GLuint attribute, const char *name)
{
    glBindAttribLocation(self->program_id, attribute, name);
}

/*Internal use only, TODO: merge with shader_dispose ?*/
static void shader_cleanup(Shader *self)
{
    if(glIsShader(self->vertex_id) == GL_TRUE)
        glDeleteShader(self->vertex_id);

    if(glIsShader(self->fragment_id) == GL_TRUE)
        glDeleteShader(self->fragment_id);

    if(glIsProgram(self->program_id) == GL_TRUE)
        glDeleteProgram(self->program_id);
}

/*
 * @brief Compile, links and loads the shader code on the GPU
 *
 * Internal use only
 *
 * @param self a Shader
 * @return true on success, false otherwise
 */
static bool shader_load(Shader *self)
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

static const char *pretty_shader_type(GLenum type)
{
    if(type == GL_VERTEX_SHADER) return "GL_VERTEX_SHADER";
    if(type == GL_FRAGMENT_SHADER) return "GL_FRAGMENT_SHADER";
    return "Unknown";
}

/*
 * @brief Creates a shader from the content of @p filename.
 *
 * INTERNAL USE ONLY
 *
 * @param shader Pointer to a location where to store the resulting
 * OpenGL handle.
 * @param type One of the types accepted by glCreateShader, mainly
 * GL_VERTEX_SHADER and GL_FRAGMENT_SHADER.
 * @return true on success, false otherwise
 */
static bool shader_compile_file(GLuint *shader, GLenum type, const char *filename)
{
    FILE *fp;
    GLint fsize;
    char *content;
    GLint rv;

    *shader = glCreateShader(type);
    if(!(*shader)){
        printf("glCreateShader failed for type %s(%d)\n",pretty_shader_type(type), type);
        printf("glError was: %d\n",glGetError());
        return false;
    }

    fp = fopen(filename, "rb");
    if(!fp){
        printf("Couldn't open file %s\n",filename);
        return false;
    }
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

/*TODO: Merge the following two error-reporting functions in one*/
/*
 * @brief Callback function for OpenGL to call where there is an error message
 * to report at the compile stage.
 */
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

/*
 * @brief Callback function for OpenGL to call where there is an error message
 * to report at the linking stage.
 */
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
