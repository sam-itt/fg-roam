#ifndef FRUSTUM_EXT_H
#define FRUSTUM_EXT_H

#include <stdbool.h>

#include <cglm/cglm.h>

#include "sg-sphere.h"

/*TODO: Have sphered into cglm exts*/
bool glm_frustum_cgsphered(vec4 frustum[6], SGSphered *sphere);
void glm_frustum_bounding_sphere(vec4 frustum[6], vec3 look_vector, vec3 cam_pos, float near, float fov_rad, vec4 sphere);
#endif /* FRUSTUM_EXT_H */
