/*
 * SPDX-FileCopyrightText: 2021 Samuel Cuella <samuel.cuella@gmail.com>
 *
 * This file is part of SoFIS - an open source EFIS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "texture.h"
#include "fgr-dirs.h"

#define N_NAMES 260
#define N_UNIQUE_FILES 169

static Texture *_store[256]; /*TODO: Array->Hash or embed in meshes*/
static unsigned char _ntextures = 0;

static char *files[] = {
    TEX_DIR"/Terrain/asphalt.png",TEX_DIR"/Terrain/gravel.png",TEX_DIR"/Terrain/water-lake.png",
    TEX_DIR"/Terrain/water-lake.png",TEX_DIR"/Terrain/water-lake.png",TEX_DIR"/Terrain/city1.png",
    TEX_DIR"/Terrain/drycrop1.png",TEX_DIR"/Terrain/irrcrop1.png",TEX_DIR"/Terrain/mixedcrop1.png",
    TEX_DIR"/Terrain/naturalcrop1.png",TEX_DIR"/Terrain/cropgrass1.png",TEX_DIR"/Terrain/cropgrass1.png",
    TEX_DIR"/Terrain/shrub1.png",TEX_DIR"/Terrain/deciduous1.png",TEX_DIR"/Terrain/forest1a.png",
    TEX_DIR"/Terrain/mixedforest.png",TEX_DIR"/Terrain/shrub1.png",TEX_DIR"/Terrain/airport.png",
    TEX_DIR"/Terrain/airport.png",TEX_DIR"/Terrain/rock.png",TEX_DIR"/Terrain/glacier3.png",
    TEX_DIR"/Terrain/golfcourse1.png",TEX_DIR"/Terrain/airport.png",TEX_DIR"/Terrain/deciduous1.png",
    TEX_DIR"/Terrain/city1.png",TEX_DIR"/Terrain/water-lake.png",TEX_DIR"/Terrain/rock.png",
    TEX_DIR"/Terrain/irrcrop1.png",TEX_DIR"/Terrain/asphalt.png",TEX_DIR"/Terrain/rock.png",
    TEX_DIR"/Terrain/Town1.png",TEX_DIR"/Terrain/gravel.png",TEX_DIR"/Terrain/irrcrop1.png",
    TEX_DIR"/Runway/lf_dbl_solid_yellow.png",TEX_DIR"/Runway/lf_runway_hold_border.png",
    TEX_DIR"/Runway/pa_0l.png",TEX_DIR"/Runway/pa_2l.png",TEX_DIR"/Runway/pa_2r.png",
    TEX_DIR"/Runway/pa_4r.png",TEX_DIR"/Runway/pa_aim.png",TEX_DIR"/Runway/pa_centerline.png",
    TEX_DIR"/Runway/pa_dspl_arrows.png",TEX_DIR"/Runway/pa_dspl_thresh.png",TEX_DIR"/Runway/pa_rest.png",
    TEX_DIR"/Runway/pa_shoulder_f1.png",TEX_DIR"/Runway/pa_threshold.png",TEX_DIR"/Runway/pc_helipad.png",
    TEX_DIR"/Runway/pc_tiedown.png",TEX_DIR"/Runway/grass_rwy.png",
    TEX_DIR"/Terrain/cropwood.png", TEX_DIR"/Terrain/asphalt.png", TEX_DIR"/Terrain/tundra.png",
    TEX_DIR"/Symbols/bidirectional.png", TEX_DIR"/Signs/black.png", TEX_DIR"/Terrain/deciduous1.png",
    TEX_DIR"/Terrain/city1.png", TEX_DIR"/Terrain/lava1.png", TEX_DIR"/Terrain/tundra.png",
    TEX_DIR"/Terrain/city1.png", TEX_DIR"/Terrain/cropwood.png", TEX_DIR"/Terrain/cropwood.png",
    TEX_DIR"/Terrain/deciduous1.png", TEX_DIR"/Terrain/dec_evergreen.png", TEX_DIR"/Terrain/forest1a.png",
    TEX_DIR"/Runway/pc_0l.png", TEX_DIR"/Runway/pc_0r.png", TEX_DIR"/Runway/pc_11.png",
    TEX_DIR"/Runway/pc_1c.png", TEX_DIR"/Runway/pc_1l.png", TEX_DIR"/Runway/pc_1r.png",
    TEX_DIR"/Runway/pc_2c.png", TEX_DIR"/Runway/pc_2l.png", TEX_DIR"/Runway/pc_2r.png",
    TEX_DIR"/Runway/pc_3c.png", TEX_DIR"/Runway/pc_3l.png", TEX_DIR"/Runway/pc_3r.png",
    TEX_DIR"/Runway/pc_4c.png", TEX_DIR"/Runway/pc_4r.png", TEX_DIR"/Runway/pc_5c.png",
    TEX_DIR"/Runway/pc_5r.png", TEX_DIR"/Runway/pc_6c.png", TEX_DIR"/Runway/pc_6r.png",
    TEX_DIR"/Runway/pc_7c.png", TEX_DIR"/Runway/pc_7r.png", TEX_DIR"/Runway/pc_8c.png",
    TEX_DIR"/Runway/pc_8r.png", TEX_DIR"/Runway/pc_9c.png", TEX_DIR"/Runway/pc_9r.png",
    TEX_DIR"/Runway/pc_aim.png", TEX_DIR"/Runway/pc_aim_uk.png", TEX_DIR"/Runway/pc_centerline.png",
    TEX_DIR"/Runway/pc_C.png", TEX_DIR"/Runway/pc_L.png", TEX_DIR"/Runway/pc_rest.png",
    TEX_DIR"/Runway/pc_R.png", TEX_DIR"/Runway/dirt_rwy.png", TEX_DIR"/Runway/pc_taxiway.png",
    TEX_DIR"/Runway/pc_threshold.png", TEX_DIR"/Runway/pc_tz_one_a.png", TEX_DIR"/Runway/pc_tz_one_b.png",
    TEX_DIR"/Runway/pc_tz_three.png", TEX_DIR"/Runway/pc_tz_two_a.png", TEX_DIR"/Runway/pc_tz_two_b.png",
    TEX_DIR"/Terrain/rock.png", TEX_DIR"/Terrain/rock.png", TEX_DIR"/Terrain/water-lake.png",
    TEX_DIR"/Terrain/forest1a.png", TEX_DIR"/Terrain/evergreen.png", TEX_DIR"/Terrain/marsh2.png",
    TEX_DIR"/Signs/framed.png", TEX_DIR"/Terrain/gravel.png", TEX_DIR"/Terrain/herbtundra.png",
    TEX_DIR"/Terrain/herbtundra.png", TEX_DIR"/Terrain/marsh2.png", TEX_DIR"/Terrain/sand1.png",
    TEX_DIR"/Terrain/forest1a.png", TEX_DIR"/Terrain/water-lake.png", TEX_DIR"/Runway/lakebed_taxiway.png",
    TEX_DIR"/Terrain/forest1a.png", TEX_DIR"/Terrain/lava1.png", TEX_DIR"/Runway/lf_broken_red_border.png",
    TEX_DIR"/Runway/lf_broken_white_border.png", TEX_DIR"/Runway/lf_broken_white.png", TEX_DIR"/Runway/lf_checkerboard_white.png",
    TEX_DIR"/Runway/lf_dbl_lane_queue_border.png", TEX_DIR"/Runway/lf_dbl_lane_queue.png", TEX_DIR"/Runway/lf_ils_hold_border.png",
    TEX_DIR"/Runway/lf_ils_hold.png", TEX_DIR"/Runway/lf_other_hold_border.png", TEX_DIR"/Runway/lf_other_hold.png",
    TEX_DIR"/Runway/lf_runway_hold.png", TEX_DIR"/Runway/lf_safetyzone_centerline_border.png", TEX_DIR"/Runway/lf_safetyzone_centerline.png",
    TEX_DIR"/Runway/lf_sng_broken_red.png", TEX_DIR"/Runway/lf_sng_broken_yellow_border.png", TEX_DIR"/Runway/lf_sng_broken_yellow.png",
    TEX_DIR"/Runway/lf_sng_lane_queue_border.png", TEX_DIR"/Runway/lf_sng_lane_queue.png", TEX_DIR"/Runway/lf_sng_solid_blue.png",
    TEX_DIR"/Runway/lf_sng_solid_green.png", TEX_DIR"/Runway/lf_sng_solid_orange.png", TEX_DIR"/Runway/lf_sng_solid_red.png",
    TEX_DIR"/Runway/lf_sng_solid_white.png", TEX_DIR"/Runway/lf_sng_solid_yellow_border.png", TEX_DIR"/Runway/lf_sng_solid_yellow.png",
    TEX_DIR"/Runway/lf_solid_blue_border.png", TEX_DIR"/Runway/lf_solid_green_border.png", TEX_DIR"/Runway/lf_solid_orange_border.png",
    TEX_DIR"/Runway/lf_solid_red_border.png", TEX_DIR"/Runway/lf_sng_solid_white_border.png", TEX_DIR"/Terrain/tidal.png",
    TEX_DIR"/Terrain/marsh2.png", TEX_DIR"/Terrain/mixedcrop1.png", TEX_DIR"/Terrain/mixedcrop1.png",
    TEX_DIR"/Terrain/tundra.png", TEX_DIR"/Terrain/water.png", TEX_DIR"/Terrain/irrcrop1.png",
    TEX_DIR"/Runway/pa_0r.png", TEX_DIR"/Runway/pa_11.png", TEX_DIR"/Runway/pa_1c.png",
    TEX_DIR"/Runway/pa_1l.png", TEX_DIR"/Runway/pa_1r.png", TEX_DIR"/Runway/pa_2c.png",
    TEX_DIR"/Runway/pa_3c.png", TEX_DIR"/Runway/pa_3l.png", TEX_DIR"/Runway/pa_3r.png",
    TEX_DIR"/Runway/pa_4c.png", TEX_DIR"/Runway/pa_5c.png", TEX_DIR"/Runway/pa_5r.png",
    TEX_DIR"/Runway/pa_6c.png", TEX_DIR"/Runway/pa_6r.png", TEX_DIR"/Runway/pa_7c.png",
    TEX_DIR"/Runway/pa_7r.png", TEX_DIR"/Runway/pa_8c.png", TEX_DIR"/Runway/pa_8r.png",
    TEX_DIR"/Runway/pa_9c.png", TEX_DIR"/Runway/pa_9r.png", TEX_DIR"/Terrain/packice1.png",
    TEX_DIR"/Runway/pa_C.png", TEX_DIR"/Runway/pa_helipad.png", TEX_DIR"/Runway/pa_L.png",
    TEX_DIR"/Runway/pa_no_threshold.png", TEX_DIR"/Runway/pa_R.png", TEX_DIR"/Runway/pa_shoulder.png",
    TEX_DIR"/Runway/pa_stopway.png", TEX_DIR"/Runway/pa_taxiway.png", TEX_DIR"/Runway/pa_tiedown.png",
    TEX_DIR"/Runway/pa_tz_one_a.png", TEX_DIR"/Runway/pa_tz_one_b.png", TEX_DIR"/Runway/pa_tz_three.png",
    TEX_DIR"/Runway/pa_tz_two_a.png", TEX_DIR"/Runway/pa_tz_two_b.png", TEX_DIR"/Runway/pc_0l.png",
    TEX_DIR"/Runway/pc_0r.png", TEX_DIR"/Runway/pc_11.png", TEX_DIR"/Runway/pc_1c.png",
    TEX_DIR"/Runway/pc_1l.png", TEX_DIR"/Runway/pc_1r.png", TEX_DIR"/Runway/pc_2c.png",
    TEX_DIR"/Runway/pc_2l.png", TEX_DIR"/Runway/pc_2r.png", TEX_DIR"/Runway/pc_3c.png",
    TEX_DIR"/Runway/pc_3l.png", TEX_DIR"/Runway/pc_3r.png", TEX_DIR"/Runway/pc_4c.png",
    TEX_DIR"/Runway/pc_4r.png", TEX_DIR"/Runway/pc_5c.png", TEX_DIR"/Runway/pc_5r.png",
    TEX_DIR"/Runway/pc_6c.png", TEX_DIR"/Runway/pc_6r.png", TEX_DIR"/Runway/pc_7c.png",
    TEX_DIR"/Runway/pc_7r.png", TEX_DIR"/Runway/pc_8c.png", TEX_DIR"/Runway/pc_8r.png",
    TEX_DIR"/Runway/pc_9c.png", TEX_DIR"/Runway/pc_9r.png", TEX_DIR"/Runway/pc_aim.png",
    TEX_DIR"/Runway/pc_aim_uk.png", TEX_DIR"/Runway/pc_centerline.png", TEX_DIR"/Runway/pc_C.png",
    TEX_DIR"/Runway/pc_dspl_arrows.png", TEX_DIR"/Runway/pc_dspl_thresh.png", TEX_DIR"/Runway/pc_L.png",
    TEX_DIR"/Runway/pc_no_threshold.png", TEX_DIR"/Runway/pc_rest.png", TEX_DIR"/Runway/pc_R.png",
    TEX_DIR"/Runway/pc_shoulder_f.png", TEX_DIR"/Runway/pc_shoulder.png", TEX_DIR"/Runway/pc_stopway.png",
    TEX_DIR"/Runway/pc_taxiway.png", TEX_DIR"/Runway/pc_threshold.png", TEX_DIR"/Runway/pc_tz_one_a.png",
    TEX_DIR"/Runway/pc_tz_one_b.png", TEX_DIR"/Runway/pc_tz_three.png", TEX_DIR"/Runway/pc_tz_two_a.png",
    TEX_DIR"/Runway/pc_tz_two_b.png", TEX_DIR"/Terrain/glacier3.png", TEX_DIR"/Terrain/water-lake.png",
    TEX_DIR"/Terrain/city1.png", TEX_DIR"/Terrain/mixedforest.png", TEX_DIR"/Signs/red.png",
    TEX_DIR"/Terrain/water-lake.png", TEX_DIR"/Terrain/irrcrop1.png", TEX_DIR"/Terrain/water-lake.png",
    TEX_DIR"/Terrain/marsh2.png", TEX_DIR"/Terrain/sand4.png", TEX_DIR"/Terrain/savanna.png",
    TEX_DIR"/Terrain/shrub1.png", TEX_DIR"/Signs/signs_case.png", TEX_DIR"/Terrain/snow1.png",
    TEX_DIR"/Terrain/forest1a.png", TEX_DIR"/Signs/special.png", TEX_DIR"/Symbols/unidirectionalgreen.png",
    TEX_DIR"/Symbols/unidirectionalred.png", TEX_DIR"/Symbols/unidirectional.png", TEX_DIR"/Terrain/unknown.png",
    TEX_DIR"/Terrain/evergreen.png", TEX_DIR"/Terrain/marsh2.png", TEX_DIR"/Signs/yellow.png",
    TEX_DIR"/Terrain/Town1.png"
};

static char *names[] = {
    "Freeway","Railroad","Stream",
    "Watercourse","Canal","Urban",
    "DryCrop","IrrCrop","ComplexCrop",
    "NaturalCrop","CropGrass","Grassland",
    "Scrub","DeciduousForest","EvergreenForest",
    "MixedForest","Sclerophyllous","Airport",
    "Grass","BarrenCover","Glacier",
    "GolfCourse","Greenspace","Heath",
    "Industrial","Lake","OpenMining",
    "Orchard","Road","Rock","Town",
    "Transport","Vineyard","lf_dbl_solid_yellow",
    "lf_runway_hold_border","pa_0l","pa_2l",
    "pa_2r","pa_4r","pa_aim",
    "pa_centerline","pa_dspl_arrows","pa_dspl_thresh",
    "pa_rest","pa_shoulder_f","pa_threshold",
    "pc_heli","pc_tiedown","grass_rwy",
    "AgroForest", "Asphalt", "BareTundraCover",
    "BidirectionalTaper", "BlackSign", "Bog",
    "BuiltUpCover", "Burnt", "Cemetery",
    "Construction", "CropWoodCover", "CropWood",
    "DeciduousBroadCover", "DeciduousNeedleCover", "Default",
    "dirt_rwy0l", "dirt_rwy0r", "dirt_rwy11",
    "dirt_rwy1c", "dirt_rwy1l", "dirt_rwy1r",
    "dirt_rwy2c", "dirt_rwy2l", "dirt_rwy2r",
    "dirt_rwy3c", "dirt_rwy3l", "dirt_rwy3r",
    "dirt_rwy4c", "dirt_rwy4r", "dirt_rwy5c",
    "dirt_rwy5r", "dirt_rwy6c", "dirt_rwy6r",
    "dirt_rwy7c", "dirt_rwy7r", "dirt_rwy8c",
    "dirt_rwy8r", "dirt_rwy9c", "dirt_rwy9r",
    "dirt_rwyaim", "dirt_rwyaim_uk", "dirt_rwycenterline",
    "dirt_rwyC", "dirt_rwyL", "dirt_rwyrest",
    "dirt_rwyR", "dirt_rwy", "dirt_rwytaxiway",
    "dirt_rwythreshold", "dirt_rwytz_one_a", "dirt_rwytz_one_b",
    "dirt_rwytz_three", "dirt_rwytz_two_a", "dirt_rwytz_two_b",
    "Dirt", "Dump", "Estuary",
    "EvergreenBroadCover", "EvergreenNeedleCover", "FloodLand",
    "FramedSign", "Gravel", "HerbTundraCover",
    "HerbTundra", "HerbWetlandCover", "IntermittentReservoir",
    "Island", "Lagoon", "lakebed_taxiway",
    "Landmass", "Lava", "lf_broken_red_border",
    "lf_broken_white_border", "lf_broken_white", "lf_checkerboard_white",
    "lf_dbl_lane_queue_border", "lf_dbl_lane_queue", "lf_ils_hold_border",
    "lf_ils_hold", "lf_other_hold_border", "lf_other_hold",
    "lf_runway_hold", "lf_safetyzone_centerline_border", "lf_safetyzone_centerline",
    "lf_sng_broken_red", "lf_sng_broken_yellow_border", "lf_sng_broken_yellow",
    "lf_sng_lane_queue_border", "lf_sng_lane_queue", "lf_sng_solid_blue",
    "lf_sng_solid_green", "lf_sng_solid_orange", "lf_sng_solid_red",
    "lf_sng_solid_white", "lf_sng_solid_yellow_border", "lf_sng_solid_yellow",
    "lf_solid_blue_border", "lf_solid_green_border", "lf_solid_orange_border",
    "lf_solid_red_border", "lf_solid_white_border", "Littoral",
    "Marsh", "MixedCropPastureCover", "MixedCrop",
    "MixedTundraCover", "Ocean", "Olives",
    "pa_0r", "pa_11", "pa_1c",
    "pa_1l", "pa_1r", "pa_2c",
    "pa_3c", "pa_3l", "pa_3r",
    "pa_4c", "pa_5c", "pa_5r",
    "pa_6c", "pa_6r", "pa_7c",
    "pa_7r", "pa_8c", "pa_8r",
    "pa_9c", "pa_9r", "PackIce",
    "pa_C", "pa_heli", "pa_L",
    "pa_no_threshold", "pa_R", "pa_shoulder",
    "pa_stopway", "pa_taxiway", "pa_tiedown",
    "pa_tz_one_a", "pa_tz_one_b", "pa_tz_three",
    "pa_tz_two_a", "pa_tz_two_b", "pc_0l",
    "pc_0r", "pc_11", "pc_1c",
    "pc_1l", "pc_1r", "pc_2c",
    "pc_2l", "pc_2r", "pc_3c",
    "pc_3l", "pc_3r", "pc_4c",
    "pc_4r", "pc_5c", "pc_5r",
    "pc_6c", "pc_6r", "pc_7c",
    "pc_7r", "pc_8c", "pc_8r",
    "pc_9c", "pc_9r", "pc_aim",
    "pc_aim_uk", "pc_centerline", "pc_C",
    "pc_dspl_arrows", "pc_dspl_thresh", "pc_L",
    "pc_no_threshold", "pc_rest", "pc_R",
    "pc_shoulder_f", "pc_shoulder", "pc_stopway",
    "pc_taxiway", "pc_threshold", "pc_tz_one_a",
    "pc_tz_one_b", "pc_tz_three", "pc_tz_two_a",
    "pc_tz_two_b", "PolarIce", "Pond",
    "Port", "RainForest", "RedSign",
    "Reservoir", "Rice", "Saline",
    "SaltMarsh", "Sand", "SavannaCover",
    "ShrubCover", "signcase", "SnowCover",
    "SomeSort", "SpecialSign", "UnidirectionalTaperGreen",
    "UnidirectionalTaperRed", "UnidirectionalTaper", "Unknown",
    "WoodedTundraCover", "WoodedWetlandCover", "YellowSign",
    "SubUrban"
};

void texture_store_shutdown(void)
{
    for(int i = 0; i < _ntextures; i++)
        texture_free(_store[i]);
}

Texture *texture_new(const char *filename, const char *name)
{
    Texture *rv;

    rv = calloc(1, sizeof(Texture));
    if(rv){
        rv->filename = strdup(filename);
        if(name)
            rv->name = strdup(name);
    }
    if(!texture_load(rv)){
        texture_free(rv);
        return NULL;
    }
    _store[_ntextures++] = rv;
    return rv;
}

void texture_free(Texture *self)
{
    if(self->filename)
        free(self->filename);
    if(self->name)
        free(self->name);
    glDeleteTextures(1, &self->id);
    free(self);
}

bool texture_load(Texture *self)
{
    SDL_Surface *img;
    GLenum internal_format;
    GLenum format;

    img = IMG_Load(self->filename);
    if(!img){
        printf("SDL_Image couldn't load %s: %s\n",self->filename,SDL_GetError());
        return false;
    }

    glGenTextures(1, &(self->id));
    glBindTexture(GL_TEXTURE_2D, self->id);

    if(img->format->BytesPerPixel == 3){
        internal_format = GL_RGB;
        if(img->format->Rmask == 0xff)
            format = GL_RGB;
        else
#if USE_GLES
            goto bail;
#else
            format = GL_BGR;
#endif
    }else if(img->format->BytesPerPixel == 4){
        internal_format = GL_RGBA;
        if(img->format->Rmask == 0xff)
            format = GL_RGBA;
        else
#if USE_GLES
            goto bail;
#else
            format = GL_BGRA;
#endif
    }else{
bail:
        printf("Unknown image format: %d Bytes per pixel\n",img->format->BytesPerPixel);
        SDL_FreeSurface(img);
        return false;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img->w, img->h, 0, format, GL_UNSIGNED_BYTE, img->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(img);
    return true;
}

Texture *texture_get_by_name(const char *name)
{
    for(int i = 0; i < _ntextures; i++){
        if(!strcmp(_store[i]->name,name))
            return _store[i];
    }

    for(int idx = 0; idx < N_NAMES; idx++){
        if(!strcmp(names[idx], name)){
            return texture_new(files[idx], name);
        }
    }
    return NULL;
}


GLuint texture_get_id_by_name(const char *name)
{
    Texture *t;

    t = texture_get_by_name(name);
/*    if(t)
        printf("%s is texture %d\n",name, t->id);
    else
        printf("%s not found\n", name);*/
    return t ? t->id : 0;
}
