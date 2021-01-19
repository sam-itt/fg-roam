#ifndef INDICE_H
#define INDICE_H
#include <SDL2/SDL_opengl.h>

#define USE_INT_INDICES 0

/* This will define the max indice value that can
 * be stored, not the max number of indices
 * OpenGL ES 2.0 wants USHORT indices and we
 * want support it
 * */
#if USE_INT_INDICES
typedef GLuint indice_t;
#define INDICE_TYPE GL_UNSIGNED_INT
#define INDICE_MAX UINT32_MAX
#else
typedef GLushort indice_t;
#define INDICE_TYPE GL_UNSIGNED_SHORT
#define INDICE_MAX UINT16_MAX
#endif


#endif /* INDICE_H */
