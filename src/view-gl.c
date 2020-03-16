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
//#include "texture.h"
#include "plane.h"


SGBinObject *terrain = NULL;
Plane *plane = NULL;

void init(void)
{
    terrain = sg_bin_object_new();
    sg_bin_object_load(terrain,"../test/btg/2990336.btg");

}

void render(void)
{
    glPushMatrix();
    if ( terrain->tris_v->len != 0 ) {
        //printf("# triangle groups\n");

        int start = 0;
        int end = 1;
        char *material;
        while ( start < terrain->tri_materials->len ) {
            // find next group
            material = g_ptr_array_index(terrain->tri_materials,start); 
           // printf("tri_materials.size: %d\n", terrain->tri_materials->len);
            while ( (end < terrain->tri_materials->len) &&
                    (!strcmp(material, g_ptr_array_index(terrain->tri_materials,end))) )
            {
                //printf("end = %d\n",end);
                end++;
            }
            //printf("group = %d to %d\n",start, end-1);
            //

            // write group headers
//            printf("\n");
//            printf("# usemtl %s\n", material);
            // write groups
            glBegin(GL_TRIANGLES);
            glColor3f( 1, 0, 0 ); // red
            for (int  i = start; i < end; ++i ) {
                GArray *tri_v = g_ptr_array_index(terrain->tris_v, i);
                GArray *tri_c = g_ptr_array_index(terrain->tris_c, i);
                GPtrArray *tri_tcs = g_ptr_array_index(terrain->tris_tcs,i);
                float tex[3];
                if(tri_v->len != 3){
                    printf("Wrong tri_v->len: %d\n",tri_v->len);
                    exit(EXIT_FAILURE);
                }
                if(tri_c->len > 0)
                    printf("Triangle %d has %d colors!\n",i,tri_c->len);
                for (int  j = 0; j < tri_v->len; ++j ) {
                    int a, b;
                    GArray *ttcs = g_ptr_array_index(tri_tcs,0);
                    a = g_array_index(tri_v, int, j);
                    //b = g_array_index(ttcs, int, j);
                    //tex[j] = b + 1;
                    SGVec3d vert = g_array_index(terrain->wgs84_nodes, SGVec3d, a);
                    glVertex3f(vert.x+terrain->gbs_center.x,
                               vert.y+terrain->gbs_center.y,
                               vert.z+terrain->gbs_center.z);
                }
            }
            glEnd();

            start = end;
            end = start + 1;
        }
    }
    glPopMatrix();
}

int handle_keyboard(SDL_KeyboardEvent *event)
{
    switch(event->keysym.sym){
        case SDLK_LEFT:
            plane->roll += 0.1;
            break;
        case SDLK_RIGHT:
            plane->roll -= 0.1;
            break;
        case SDLK_DOWN:
            plane->pitch += 1.0;
            break;
        case SDLK_UP:
            plane->pitch -= 1.0;
            break;
        case SDLK_PAGEUP:
            plane->yaw -= 0.1;
            break;
        case SDLK_PAGEDOWN:
            plane->yaw += 0.1;
            break;
        case SDLK_KP_4:
            plane->X += 1.0;
            break;
        case SDLK_KP_6:
            plane->X -= 1.0;
            break;
        case SDLK_KP_8:
            plane->Y += 1.0;
            break;
        case SDLK_KP_2:
            plane->Y -= 1.0;
            break;
        case SDLK_KP_9:
            plane->Z += 1.0;
            break;
        case SDLK_KP_3:
            plane->Z -= 1.0;
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



    init();
    plane = calloc(1, sizeof(Plane));
    plane->X =4741964.50000;
    plane->Y =185755.81250;
    plane->Z =4247972.00000;

    while(!done){
        done = handle_event();

        glClearColor (1.0, 1.0, 1.0, 0.0);
        glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity ();
        gluPerspective(60.0,800/600,1.0,100.0);

        glMatrixMode (GL_MODELVIEW);
        glLoadIdentity ();

        PlaneView(plane);

        render();
        SDL_Delay(10);
        SDL_GL_SwapWindow(window);
    }


    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

	exit(EXIT_SUCCESS);
}
