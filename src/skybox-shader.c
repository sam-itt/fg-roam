#include <stdio.h>
#include <stdlib.h>

#include "skybox-shader.h"

SkyboxShader *skybox_shader_new(void)
{
    SkyboxShader *rv;

    rv = calloc(1, sizeof(SkyboxShader));
    if(rv){
        if(!skybox_shader_init(rv)){
            free(rv);
            return NULL;
        }
    }
    return rv;
}

SkyboxShader *skybox_shader_init(SkyboxShader *self)
{
    bool rv;

    if(!shader_init(SHADER(self), SHADER_DIR"/sky-vertex.gl", SHADER_DIR"/sky-fragment.gl"))
        return NULL;

    rv = shader_get_uniform_locationp(SHADER(self), "projectionMatrix", &(self->projection_matrix));
    rv &= shader_get_uniform_locationp(SHADER(self), "viewMatrix", &(self->view_matrix));
    rv &= shader_get_uniform_locationp(SHADER(self), "cubeMap", &(self->texunit));
    rv &= shader_get_attribute_locationp(SHADER(self), "position", &(self->position));

    if(rv)
        return self;

    skybox_shader_dispose(self);
    return NULL;
}

void skybox_shader_dispose(SkyboxShader *self)
{
    shader_dispose(SHADER(self));
}

void skybox_shader_free(SkyboxShader *self)
{
    skybox_shader_dispose(self);
    free(self);
}

