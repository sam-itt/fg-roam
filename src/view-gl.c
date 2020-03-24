#define _GNU_SOURCE 1 
#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <cglm/cglm.h>

#include "btg-io.h"
#include "shader.h"
#include "texture.h"
#include "plane.h"
#include "mesh.h"
#include "bucket.h"
#include "geodesy.h"
#include "misc.h"
#include "tile-manager.h"

Plane *plane = NULL;
Mesh *mesh = NULL;
Shader *shader = NULL;

GLint a_position = 0;
GLint a_texcoords = 0;
GLint u_tex = 0;
GLint u_mvpmtx = 0;


void render(double vis)
{
    SGBucket **buckets;
    Mesh *m;
    double lat, lon, height;
    SGVec3d epos;

    plane_get_position(plane, &lat, &lon, &height);
    buckets = tile_manager_get_tiles(tile_manager_get_instance(), lat, lon, 1.0);
    
    epos = (SGVec3d){plane->X,plane->Y,plane->Z};

    for(int i = 0; buckets[i] != NULL; i++){
        m = sg_bucket_get_mesh(buckets[i]);
            if(m){
            //   mesh_render(m, &epos, vis);
                mesh_render_buffer(m, a_position, a_texcoords);
            }
    }
#if 0
    for(SGBucket *b = *buckets; *buckets != NULL; b++){
        m = sg_bucket_get_mesh(b);
            if(m)
                mesh_render(m);
    }
#endif
}

int handle_keyboard(SDL_KeyboardEvent *event)
{
    switch(event->keysym.sym){
        case SDLK_LEFT:
            plane->vroll = (event->state == SDL_PRESSED) ? 0.1 : 0;
            break;
        case SDLK_RIGHT:
            plane->vroll = (event->state == SDL_PRESSED) ? -0.1 : 0;
            break;
        case SDLK_DOWN:
            plane->vpitch = (event->state == SDL_PRESSED) ? 1.0 : 0;
            break;
        case SDLK_UP:
            plane->vpitch = (event->state == SDL_PRESSED) ? -1.0 : 0;
            break;
        case SDLK_PAGEUP:
            plane->vyaw = (event->state == SDL_PRESSED) ? -0.1 : 0;
            break;
        case SDLK_PAGEDOWN:
            plane->vyaw = (event->state == SDL_PRESSED) ? 0.1 : 0;
            break;
        case SDLK_KP_4:
            plane->vX = (event->state == SDL_PRESSED) ? 1.0 : 0;
            break;
        case SDLK_KP_6:
            plane->vX = (event->state == SDL_PRESSED) ? -1.0 : 0;
            break;
        case SDLK_KP_8:
            plane->vY = (event->state == SDL_PRESSED) ? 1.0 : 0;
            break;
        case SDLK_KP_2:
            plane->vY = (event->state == SDL_PRESSED) ? -1.0 : 0;
            break;
        case SDLK_KP_9:
            plane->vZ = (event->state == SDL_PRESSED) ? 1.0 : 0;
            break;
        case SDLK_KP_3:
            plane->vZ = (event->state == SDL_PRESSED) ? -1.0 : 0;
            break;
        case SDLK_p:
            DumpPlane(plane);
            break;
        case SDLK_ESCAPE:
            return true;
            break;
    }
    return false;
}


int handle_event(void)
{
    SDL_Event event;
    SDL_WaitEvent(&event);

    switch(event.type){
        case SDL_WINDOWEVENT:
            if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                return true;
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            return handle_keyboard(&(event.key));
            break;
    }
    return false;
}

int main(int argc, char **argv)
{

    SDL_Window* window;
    SDL_GLContext gl_context;

    bool done = false;


    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("SDL_Init error: %s\n",SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(
        "SDL Test", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 800, 600, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    if(!window){
        printf("Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }


    gl_context = SDL_GL_CreateContext(window);

    if(!gl_context){
        printf("Couldn't create context: %s\n",SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();

        exit(EXIT_FAILURE);
    }




    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);


    texture_new("../../textures/asphalt.png","Freeway");
    texture_new("../../textures/gravel.png","Railroad");
    texture_new("../../textures/water-lake.png","Stream");
    texture_new("../../textures/water-lake.png","Watercourse");
    texture_new("../../textures/water-lake.png","Canal");
    texture_new("../../textures/city1.png","Urban");
    texture_new("../../textures/drycrop1.png","DryCrop");
    texture_new("../../textures/irrcrop1.png","IrrCrop");
    texture_new("../../textures/mixedcrop1.png","ComplexCrop");
    texture_new("../../textures/naturalcrop1.png","NaturalCrop");
    texture_new("../../textures/cropgrass1.png","CropGrass");
    texture_new("../../textures/cropgrass1.png","Grassland");
    texture_new("../../textures/shrub1.png","Scrub");
    texture_new("../../textures/deciduous1.png","DeciduousForest");
    texture_new("../../textures/forest1a.png","EvergreenForest");
    texture_new("../../textures/mixedforest.png","MixedForest");
    texture_new("../../textures/shrub1.png","Sclerophyllous");

    plane = calloc(1, sizeof(Plane));
    plane->X = 4742006.50000;
    plane->Y = 185376.81250;
    plane->Z = 4248252.00000;
    
    plane->roll = 79.69936;
    plane->pitch = -9.00000;
    plane->yaw = 16.80003;
    plane->bearing = NAN;

    shader = shader_new("vertex.gl","fragment.gl");
    a_position = shader_get_attribute_location(shader, "position");
    a_texcoords = shader_get_attribute_location(shader, "texcoord");
    printf("a_position: %d, a_texcoords: %d\n",a_position,a_texcoords);
    u_tex = shader_get_uniform_location(shader, "texture");
    if(a_texcoords < 0){
        printf("Cannot bind a_texcoords\n");
        exit(-1);
    }
#if !USE_OGL_MTX
    u_mvpmtx = shader_get_uniform_location(shader, "mvp");
    if(u_mvpmtx < 0){
        printf("Cannot bind u_mvpmtx\n");
        exit(-1);
    }
#endif


    mat4 projection_matrix;
    mat4 mvp;

    glm_mat4_identity(projection_matrix);
    glm_perspective(glm_rad(60.0f), 800.0f/600.0f, 1.0f, 1000.0f, projection_matrix);


    Uint32 ticks;
    Uint32 last_ticks = 0; 
    Uint32 elapsed = 0; 
    Uint32 acc = 0; 
    Uint32 nframes = 0;

    while(!done){
        ticks = SDL_GetTicks();

        done = handle_event();

        glClearColor (1.0, 1.0, 1.0, 0.0);
        glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
#if USE_OGL_MTX
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        gluPerspective(60.0,800/600,1.0,1000.0);

        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();
#elif 0
        glm_mat4_identity(projection_matrix);
        glm_perspective(glm_rad(60.0f), 800.0/600.0, 1.0f, 1000.0f, projection_matrix);
#endif
        glUseProgram(shader->program_id);
        glUniform1i(u_tex, 0);

        PlaneView(plane);
#if !USE_OGL_MTX
        glm_mat4_identity(mvp);
        glm_mat4_mul(projection_matrix, plane->view, mvp);
        glUniformMatrix4fv(u_mvpmtx, 1, GL_FALSE, mvp[0]);
#endif
        render(10000.0);
        SDL_GL_SwapWindow(window);
        nframes++;
        elapsed = ticks - last_ticks;
        acc += elapsed;
        if(elapsed < 20){
            SDL_Delay(20 - elapsed);
        }
        if(acc >= 4000){ /*1sec*/
            plane_show_position(plane);

            printf("Current FPS: %d\n",nframes);
            nframes = 0;
            acc = 0;
        }
        last_ticks = ticks;
    }
    shader_free(shader);
    tile_manager_shutdown();
    free(plane);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

	exit(EXIT_SUCCESS);
}
