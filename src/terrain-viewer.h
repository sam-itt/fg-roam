#ifndef TERRAIN_VIEWER_H
#define TERRAIN_VIEWER_H

#include <cglm/cglm.h>

#include "basic-shader.h"
#include "mesh.h"
#include "plane.h"
#include "skybox.h"

typedef struct{
    BasicShader *shader;
    Plane *plane; /*This is more a camera*/

    Mesh *lflg; /*Ad-hoc and Temporary*/
    Skybox *skybox; /*Might get rid of it*/

    bool dirty;
    /*TODO: Put that in  plane/camera class?*/
    float fov_rad;
    float near_plane;
    /*Matrices*/
    mat4d projection;
    mat4d projection_view;

    /*TODO: Look into using doubles*/
    vec4 fplanes[6]; /*frustrum planes*/
    vec4 frustrum_bs; /*frustrum bounding sphere*/
}TerrainViewer;


TerrainViewer *terrain_viewer_new(void);
TerrainViewer *terrain_viewer_init(TerrainViewer *self);;
TerrainViewer *terrain_viewer_dispose(TerrainViewer *self);
TerrainViewer *terrain_viewer_free(TerrainViewer *self);

void terrain_viewer_update_plane(TerrainViewer *self, double lat, double lon, double alt, double roll, double pitch, double heading);
void terrain_viewer_frame(TerrainViewer *self);
#endif /* TERRAIN_VIEWER_H */
