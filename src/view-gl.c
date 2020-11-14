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
#include "texture.h"
#include "plane.h"
#include "mesh.h"
#include "bucket.h"
#include "geodesy.h"
#include "misc.h"
#include "tile-manager.h"
#include "gps-file-feed.h"

#include "skybox.h"
#include "basic-shader.h"

#if 0
#include "flightgear-connector.h"
#else
#include "fg-tape.h"

typedef struct __attribute__((__packed__))  {
    double latitude;
    double longitude;
    double altitude;
    float roll;
    float pitch;
    float heading;
}TpBuffer;

#endif

Plane *plane = NULL;
Mesh *mesh = NULL;

unsigned int global_accelerator = 1;

Mesh *lflg = NULL;

void render(double vis, BasicShader *shader, mat4d mv)
{
    SGBucket **buckets;
    Mesh *m;

    buckets = tile_manager_get_tiles(tile_manager_get_instance(), plane->lat, plane->lon, 1.0);
    glUseProgram(SHADER(shader)->program_id);
    for(int i = 0; buckets[i] != NULL; i++){
        m = sg_bucket_get_mesh(buckets[i]);
            if(m){
                mesh_render_buffer(m, shader, mv);
            }
    }
    if(lflg)
        mesh_render_buffer(lflg, shader, mv);
    glUseProgram(0);
}

int handle_keyboard(SDL_KeyboardEvent *event)
{
    switch(event->keysym.sym){
        case SDLK_LEFT:
            plane->vroll = (event->state == SDL_PRESSED) ? 1.0 : 0;
            break;
        case SDLK_RIGHT:
            plane->vroll = (event->state == SDL_PRESSED) ? -1.0 : 0;
            break;
        case SDLK_DOWN:
            plane->vpitch = (event->state == SDL_PRESSED) ? 1.0 : 0;
            break;
        case SDLK_UP:
            plane->vpitch = (event->state == SDL_PRESSED) ? -1.0 : 0;
            break;
        case SDLK_PAGEUP:
            plane->vheading = (event->state == SDL_PRESSED) ? -1.0 : 0;
            break;
        case SDLK_PAGEDOWN:
            plane->vheading = (event->state == SDL_PRESSED) ? 1.0 : 0;
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
        case SDLK_a:
            global_accelerator++;
            printf("global_accelerator: %d\n",global_accelerator);
            break;
        case SDLK_q:
            if(global_accelerator > 1)
                global_accelerator--;
            printf("global_accelerator: %d\n",global_accelerator);
            break;
        case SDLK_n:
            plane->speed = (event->state == SDL_PRESSED) ? 1.0 : 0;
            break;
        case SDLK_b:
            plane->speed = (event->state == SDL_PRESSED) ? -1.0 : 0;
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
#if ENABLE_WAIT_EVENT
    SDL_WaitEvent(&event);
#else
    while (SDL_PollEvent(&event)) {
#endif
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
#if !ENABLE_WAIT_EVENT
    }
#endif
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

    lflg = mesh_new_from_file("../../Terrain/e000n40/e005n45/LFLG.btg.gz");

    plane = plane_new(); /*implicit  0 0 0 yaw pitch roll*/
    //plane_set_position(plane, 44.451950000, 5.726316667, 852);
    plane_set_position(plane, 45.21547, 5.84483, 718/3.281 + 4);
    DumpPlane(plane);

    BasicShader *bshader;

    bshader = basic_shader_new();
    if(!bshader){
        printf("Couldn't create mandatory BasicShader, bailing out\n");
        exit(-1);
    }

    mat4d projection_matrix;
    mat4d proj_view;

    glm_mat4d_identity(projection_matrix);
    glm_perspectived(glm_rad(60.0), 800.0/600.0, 1.0, 1000.0, projection_matrix);

    Skybox *skybox = skybox_new(projection_matrix);
    mat4d skyview = GLM_MAT4D_IDENTITY_INIT;

    Uint32 ticks;
    Uint32 last_ticks = 0;
    Uint32 elapsed = 0;
    Uint32 acc = 0;
    Uint32 nframes = 0;

    time_t start = time(NULL);
    time_t dt;

#if 0
    FlightgearConnector *fglink;
    fglink = flightgear_connector_new(6789);
    flightgear_connector_set_nonblocking(fglink);
    FlightgearPacket packet;
#else
    FGTape *tape;
    FGTapeSignal signals[6];
    TpBuffer tbuffer;

    tape = fg_tape_new_from_file("../lib/fg-io/fg-tape/dr400.fgtape");
    fg_tape_get_signals(tape, signals,
        "/position[0]/latitude-deg[0]",
        "/position[0]/longitude-deg[0]",
        "/position[0]/altitude-ft[0]",
        "/orientation[0]/roll-deg[0]",
        "/orientation[0]/pitch-deg[0]",
        "/orientation[0]/heading-deg[0]",
        NULL
    );
#endif
    time_t printed;
    Uint32 startms = SDL_GetTicks();
    Uint32 dtms;
    Uint32 last_dtms;
    int start_pos = 120;
//    start_pos = 0;

    while(!done){
        ticks = SDL_GetTicks();
        elapsed = ticks - last_ticks;
        dt = time(NULL) - start + start_pos;
        dtms = ticks - startms + (start_pos * 1000.0);
        dt *= global_accelerator;

        done = handle_event();
#if 0
        if(flightgear_connector_get_packet(fglink, &packet)){
            float lon = fmod(packet.longitude+180, 360.0) - 180;
            packet.altitude = roundf(packet.altitude/3.281);
            float calt = packet.altitude + 1.5/*+ 398*/;
            calt = roundf(calt * 100) / 100.0;
    //        packet.roll = roundf(packet.roll * 100) / 100.0;
    //        packet.pitch = roundf(packet.pitch * 100) / 100.0;
    //        packet.heading = roundf(packet.heading * 100) / 100.0;

//            printf("Packet altitude: %d feets, %0.2f meters, corrected to %0.2f meters\n", packet.altitude, packet.altitude/3.281,calt);
//            plane_update_position(plane, packet.latitude, lon, packet.altitude + 2, elapsed);
            plane_set_position(plane, packet.latitude, lon, packet.altitude + 2);
            plane_set_attitude(plane, packet.roll, packet.pitch, packet.heading);
        }
#else
        if(dtms - last_dtms >= (1000/25)){ //One update per 1/25 second
            fg_tape_get_data_at(tape, dtms / 1000.0, 6, signals, &tbuffer);

    //            printf("Packet altitude: %d feets, %0.2f meters, corrected to %0.2f meters\n", packet.altitude, packet.altitude/3.281,calt);
    //            plane_update_position(plane, packet.latitude, lon, packet.altitude + 2, elapsed);
            float lon = fmod(tbuffer.longitude+180, 360.0) - 180;
            tbuffer.altitude = tbuffer.altitude/3.281;
            plane_set_position(plane, tbuffer.latitude, lon, tbuffer.altitude+2);
            plane_set_attitude(plane, tbuffer.roll, tbuffer.pitch, tbuffer.heading);

            last_dtms = dtms;
        }
//        printf("dt: %ld seconds\n", dt);
#endif

        glClearColor (1.0, 1.0, 1.0, 0.0);
        glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


//        plane_update(plane, GPS_FEED(feed));
//        plane_update_timed(plane, feed, dt);
        PlaneView(plane, MSEC_TO_SEC(elapsed));

        glm_mat4d_identity(proj_view);
        glm_mat4d_mul(projection_matrix, plane->view, proj_view);

        glm_mat4d_identity(skyview);
//        glm_rotate_y(skyview, glm_rad(-plane->heading), skyview);
//        glm_rotate_x(skyview, glm_rad(plane->pitch), skyview);

        glEnable(GL_DEPTH_TEST);   // skybox should be drawn behind anything else
        render(10000.0, bshader, proj_view);
        skybox_render(skybox, skyview);

        SDL_GL_SwapWindow(window);
        nframes++;
        acc += elapsed;
        if(elapsed < 20){
            SDL_Delay(20 - elapsed);
        }
        if(acc >= 1000){ /*1sec*/
            int h,m,s;

            h = dtms/3600000;
            dtms -= dtms/3600000 * h;
            m = dtms / 60000;
            dtms -= 60000 * m;
            s = dtms / 1000;

            printf("%02d:%02d:%02d Current FPS: %03d\r",h,m,s, (1000*nframes)/elapsed);
            fflush(stdout);
            nframes = 0;
            acc = 0;
        }
        last_ticks = ticks;
    }
    basic_shader_free(bshader);
    tile_manager_shutdown();
    plane_free(plane);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

	exit(EXIT_SUCCESS);
}
