#ifndef QUAT_EXT_H
#define QUAT_EXT_H
#include <cglm/cglm.h>

void glm_quat_from_euler(versor q, double z, double y, double x);
void glm_quat_from_lon_lat(versor q, double lon, double lat);
void glm_quat_from_ypr(versor q, float yaw, float pitch, float roll);
#endif /* QUAT_EXT_H */
