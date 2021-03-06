/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#define _GNU_SOURCE 1
#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>

#include <cglm/cglm.h>
#if USE_GLES
#include <SDL2/SDL_opengles2.h>
#include <SDL_opengles2_gl2ext.h>
#else
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#endif

#include "terrain-viewer.h"

#include "tile-manager.h"
#include "cglm/mat4d.h"
#include "mesh.h"
#include "frustum-ext.h"

#if ENABLE_DEBUG_TRIANGLE
#include "debug-triangle.h"
#elif ENABLE_DEBUG_CUBE
#include "debug-cube.h"
#endif

TerrainViewer *terrain_viewer_new(float obliqueness)
{
    TerrainViewer *rv;

    rv = calloc(1, sizeof(TerrainViewer));
    if(rv){
        if(!terrain_viewer_init(rv, obliqueness)){
            free(rv);
            return NULL;
        }
    }
    return rv;
}

TerrainViewer *terrain_viewer_init(TerrainViewer *self, float obliqueness)
{

    self->plane = plane_new(); /*implicit  0 0 0 yaw pitch roll*/
    if(!self->plane)
        return NULL;

    self->shader = basic_shader_new();
    if(!self->shader){
        printf("Couldn't create mandatory BasicShader, bailing out\n");
        return NULL;
    }

    /*TODO: Pack that up into camera/plane class*/
    /*FG seems to be using fov:55° and far:15km*/
    self->fov_rad = glm_rad(60.0);
    self->near_plane= 1.0;
    glm_mat4d_identity(self->projection);
    glm_perspectived(self->fov_rad, 800.0/600.0, self->near_plane, 10000.0, self->projection);

    self->skybox = skybox_new(self->projection);
    /* Make the horizon higher than the default half screen
     * https://docs.unity3d.com/Manual/ObliqueFrustum.html
     */
    self->projection[2][1] = obliqueness;

#if ENABLE_DEBUG_TRIANGLE
    self->triangle = debug_triangle_new();
    if(!self->triangle)
        return NULL;
#elif ENABLE_DEBUG_CUBE
    self->cube = debug_cube_new();
    if(!self->cube)
        return NULL;
#endif

    return self;
}

TerrainViewer *terrain_viewer_dispose(TerrainViewer *self)
{
    if(self->skybox)
        skybox_free(self->skybox);
    if(self->shader)
        basic_shader_free(self->shader);
    if(self->plane)
        plane_free(self->plane);
#if ENABLE_DEBUG_TRIANGLE
    if(!self->triangle)
        debug_triangle_free(self->triangle);
#elif ENABLE_DEBUG_CUBE
    if(!self->cube)
        debug_cube_free(self->cube);
#endif
    tile_manager_shutdown();
    return NULL;
}

TerrainViewer *terrain_viewer_free(TerrainViewer *self)
{
    terrain_viewer_dispose(self);
    free(self);
    return NULL;
}

/*degrees, meters*/
void terrain_viewer_update_plane(TerrainViewer *self, double lat, double lon, double alt, double roll, double pitch, double heading)
{
    plane_set_position(self->plane, lat, lon, alt);
    plane_set_attitude(self->plane, roll, pitch, heading);

    self->dirty = true;
}

void terrain_viewer_frame(TerrainViewer *self)
{
    SGBucket **buckets;

#if ENABLE_DEBUG_TRIANGLE
    debug_triangle_render(self->triangle);
    return;
#elif ENABLE_DEBUG_CUBE
    glEnable(GL_DEPTH_TEST);   // skybox should be drawn behind anything else
    glDepthFunc(GL_LESS);
    debug_cube_render(self->cube);
    return;
#endif

    if(self->plane->dirty){
        plane_view(self->plane);

//        glm_mat4_identity(self->skybox->view);
 //       glm_rotate_y(self->skybox->view, glm_rad(-self->plane->heading), self->skybox->view);
//        glm_rotate_x(self->skybox->view, glm_rad(self->plane->pitch), self->skybox->view);
    }

    /* TODO: refactor this to be relying on plane/camera dirtyness?
     * this double self+plane dirty flags are confusing
     * */
    if(self->dirty){
        glm_mat4d_identity(self->projection_view);
        glm_mat4d_mul(self->projection, self->plane->view, self->projection_view);

        mat4 tmp;
        glm_mat4d_ucopyf(self->projection_view, tmp);
        glm_frustum_planes(tmp, self->fplanes);

        vec3 cam_pos = {self->plane->X,self->plane->Y, self->plane->Z};
        vec3 lookv = {self->plane->view[0][2],self->plane->view[1][2],self->plane->view[2][2]};
        glm_frustum_bounding_sphere(self->fplanes, lookv, cam_pos, self->near_plane, self->fov_rad, self->frustrum_bs);

        self->dirty = false;
    }

    glEnable(GL_DEPTH_TEST);   // skybox should be drawn behind anything else


    buckets = tile_manager_get_tiles(tile_manager_get_instance(), &(self->plane->geopos), 10000); /*10 km*/
    glUseProgram(SHADER(self->shader)->program_id);
    for(int i = 0; buckets[i] != NULL; i++){
        Mesh *iter;
        for(iter = sg_bucket_get_mesh(buckets[i]); iter != NULL; iter = iter->next){
            mesh_render_buffer(iter, self->shader, self->projection_view, self->fplanes, self->frustrum_bs);
        }

    }
    glUseProgram(0);

    skybox_render(self->skybox);
}

