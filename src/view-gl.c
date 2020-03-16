#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "btg-io.h"
//#include "shader.h"
#include "texture.h"
#include "plane.h"
#include "mesh.h"

SGBinObject *terrain = NULL;
Plane *plane = NULL;
Mesh *mesh = NULL;


void init(void)
{
    mesh = load_terrain("../test/btg/2990336.btg");
}

void render(void)
{
   mesh_render(mesh);
   return;
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


    texture_new("../textures/asphalt.png","Freeway");
    texture_new("../textures/gravel.png","Railroad");
    texture_new("../textures/water-lake.png","Stream");
    texture_new("../textures/water-lake.png","Watercourse");
    texture_new("../textures/water-lake.png","Canal");
    texture_new("../textures/city1.png","Urban");
    texture_new("../textures/drycrop1.png","DryCrop");
    texture_new("../textures/irrcrop1.png","IrrCrop");
    texture_new("../textures/mixedcrop1.png","ComplexCrop");
    texture_new("../textures/naturalcrop1.png","NaturalCrop");
    texture_new("../textures/cropgrass1.png","CropGrass");
    texture_new("../textures/cropgrass1.png","Grassland");
    texture_new("../textures/shrub1.png","Scrub");
    texture_new("../textures/deciduous1.png","DeciduousForest");
    texture_new("../textures/forest1a.png","EvergreenForest");
    texture_new("../textures/mixedforest.png","MixedForest");
    texture_new("../textures/shrub1.png","Sclerophyllous");

    init();
    plane = calloc(1, sizeof(Plane));
    plane->X = 4742006.50000;
    plane->Y = 185376.81250;
    plane->Z = 4248252.00000;
    
    plane->roll = 79.69936;
    plane->pitch = -9.00000;
    plane->yaw = 16.80003;

    while(!done){
        done = handle_event();

        glClearColor (1.0, 1.0, 1.0, 0.0);
        glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        gluPerspective(60.0,800/600,1.0,1000.0);

        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();

        PlaneView(plane);

        render();
        SDL_Delay(20);
        SDL_GL_SwapWindow(window);
    }


    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

	exit(EXIT_SUCCESS);
}
