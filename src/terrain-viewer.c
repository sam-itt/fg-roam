#define _GNU_SOURCE 1
#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <cglm/cglm.h>

#include "terrain-viewer.h"

#include "tile-manager.h"
#include "cglm/mat4d.h"
#include "mesh.h"
#include "frustum-ext.h"

TerrainViewer *terrain_viewer_new(void)
{
    TerrainViewer *rv;

    rv = calloc(1, sizeof(TerrainViewer));
    if(rv){
        if(!terrain_viewer_init(rv)){
            free(rv);
            return NULL;
        }
    }
    return rv;
}

TerrainViewer *terrain_viewer_init(TerrainViewer *self)
{

    self->plane = plane_new(); /*implicit  0 0 0 yaw pitch roll*/
    if(!self->plane)
        return NULL;

    self->lflg = mesh_new_from_file(TERRAIN_ROOT"/e000n40/e005n45/LFLG.btg.gz");
    if(!self->lflg)
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

    return self;
}

TerrainViewer *terrain_viewer_dispose(TerrainViewer *self)
{
    /*Doesn't exists yet*/
#if 0
    if(self->skybox)
    skybox_free(self->skybox);
#endif
    if(self->shader)
        basic_shader_free(self->shader);
    if(self->lflg)
        mesh_free(self->lflg);
    if(self->plane)
        plane_free(self->plane);

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
    Mesh *m;

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
        m = sg_bucket_get_mesh(buckets[i]);
        if(m){
            mesh_render_buffer(m, self->shader, self->projection_view, self->fplanes, self->frustrum_bs);
        }
    }
    if(self->lflg)
        mesh_render_buffer(self->lflg, self->shader, self->projection_view, self->fplanes, self->frustrum_bs);
    glUseProgram(0);

    skybox_render(self->skybox);
}

