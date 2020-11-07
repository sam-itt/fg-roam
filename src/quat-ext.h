#ifndef QUAT_EXT_H
#define QUAT_EXT_H
#include <cglm/cglm.h>

void glm_quatd_from_euler(versord q, double z, double y, double x);
void glm_quatd_from_lon_lat(versord q, double lon, double lat);
void glm_quatd_from_ypr(versord q, double yaw, double pitch, double roll);
#endif /* QUAT_EXT_H */
