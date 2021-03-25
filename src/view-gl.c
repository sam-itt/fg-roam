/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "SDL_timer.h"
#define _GNU_SOURCE 1
#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#if USE_GLES
#include <SDL2/SDL_opengles2.h>
#include <SDL_opengles2_gl2ext.h>
#else
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>
#endif

#include <cglm/cglm.h>

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
#include "terrain-viewer.h"


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

int handle_keyboard(SDL_KeyboardEvent *event, TerrainViewer *viewer)
{
    Plane *plane;

    plane = viewer->plane;
    switch(event->keysym.sym){
        case SDLK_LEFT:
            plane->vroll = (event->state == SDL_PRESSED) ? 1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_RIGHT:
            plane->vroll = (event->state == SDL_PRESSED) ? -1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_DOWN:
            plane->vpitch = (event->state == SDL_PRESSED) ? 1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_UP:
            plane->vpitch = (event->state == SDL_PRESSED) ? -1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_PAGEUP:
            plane->vheading = (event->state == SDL_PRESSED) ? -1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_PAGEDOWN:
            plane->vheading = (event->state == SDL_PRESSED) ? 1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_KP_4:
            plane->vX = (event->state == SDL_PRESSED) ? 1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_KP_6:
            plane->vX = (event->state == SDL_PRESSED) ? -1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_KP_8:
            plane->vY = (event->state == SDL_PRESSED) ? 1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_KP_2:
            plane->vY = (event->state == SDL_PRESSED) ? -1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_KP_9:
            plane->vZ = (event->state == SDL_PRESSED) ? 1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_KP_3:
            plane->vZ = (event->state == SDL_PRESSED) ? -1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_n:
            plane->speed = (event->state == SDL_PRESSED) ? 1.0 : 0;
            plane->dirty = true;
            break;
        case SDLK_b:
            plane->speed = (event->state == SDL_PRESSED) ? -1.0 : 0;
            plane->dirty = true;
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


int handle_event(TerrainViewer *viewer)
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
            return handle_keyboard(&(event.key), viewer);
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
#if USE_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
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

    TerrainViewer *viewer;
    viewer = terrain_viewer_new(-0.25);

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
    Uint32 ticks;
    Uint32 last_ticks = 0;
    Uint32 elapsed = 0;

    Uint32 startms;
    Uint32 dtms;
    Uint32 last_dtms;
    int start_pos = 120; /*Starting position in the tape*/
//    start_pos = 0;

    Uint32 acc = 0;
    Uint32 nframes = 0;

    Uint32 tframe_acc = 0;
    Uint32 tframe_start;
    Uint32 ntframes = 0;

    startms = SDL_GetTicks();
    while(!done){
        ticks = SDL_GetTicks();
        elapsed = ticks - last_ticks;
        dtms = ticks - startms + (start_pos * 1000.0);

        done = handle_event(viewer);
#if 0
        if(flightgear_connector_get_packet(fglink, &packet)){
            float lon = fmod(packet.longitude+180, 360.0) - 180;
            packet.altitude = packet.altitude/3.281;
            terrain_viewer_update_plane(viewer,
                packet.latitude, packet.longitude, packet.altitude + 2,
                packet.roll, packet.pitch, packet.heading
            );
        }
#else
        if(dtms - last_dtms >= (1000/25)){ //One update per 1/25 second
            fg_tape_get_data_at(tape, dtms / 1000.0, 6, signals, &tbuffer);

            float lon = fmod(tbuffer.longitude+180, 360.0) - 180;
            tbuffer.altitude = tbuffer.altitude/3.281;
            terrain_viewer_update_plane(viewer,
                tbuffer.latitude, tbuffer.longitude, tbuffer.altitude + 2,
                tbuffer.roll, tbuffer.pitch, tbuffer.heading
            );
            last_dtms = dtms;
        }
#endif

        glClearColor (1.0, 1.0, 1.0, 0.0);
        glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        tframe_start = SDL_GetTicks();
        terrain_viewer_frame(viewer);
        tframe_acc += (SDL_GetTicks() - tframe_start);
        ntframes++;

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

            printf("%02d:%02d:%02d Current FPS: %05d\r",h,m,s, (1000*nframes)/elapsed);
            fflush(stdout);
            nframes = 0;
            acc = 0;
        }
        last_ticks = ticks;
    }
    printf("Average terrain_viewer_frame duration: %f ms (%d calls)\n",(tframe_acc*1.0)/ntframes,ntframes);
    terrain_viewer_free(viewer);
    texture_store_shutdown();
    fg_tape_free(tape);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

	exit(EXIT_SUCCESS);
}
