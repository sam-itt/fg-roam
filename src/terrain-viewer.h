#ifndef TERRAIN_VIEWER_H
#define TERRAIN_VIEWER_H

#include <cglm/cglm.h>

#include "basic-shader.h"
#include "mesh.h"
#include "plane.h"
#include "skybox.h"

#if ENABLE_DEBUG_TRIANGLE
#include "debug-triangle.h"
#elif ENABLE_DEBUG_CUBE
#include "debug-cube.h"
#endif


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

#if ENABLE_DEBUG_TRIANGLE
    DebugTriangle *triangle;
#elif ENABLE_DEBUG_CUBE
    DebugCube *cube;
#endif
}TerrainViewer;


TerrainViewer *terrain_viewer_new(float obliqueness);
TerrainViewer *terrain_viewer_init(TerrainViewer *self, float obliqueness);
TerrainViewer *terrain_viewer_dispose(TerrainViewer *self);
TerrainViewer *terrain_viewer_free(TerrainViewer *self);

void terrain_viewer_update_plane(TerrainViewer *self, double lat, double lon, double alt, double roll, double pitch, double heading);
void terrain_viewer_frame(TerrainViewer *self);
#endif /* TERRAIN_VIEWER_H */
