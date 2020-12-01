#define GL_VERSION_2_1
#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include "texture.h"

#ifndef TEX_ROOT
#define TEX_ROOT "/home/samuel/dev/textures"
#endif

static Texture *_store[256]; /*TODO: Array->Hash or embed in meshes*/
static unsigned char _ntextures = 0;

static char *files[] = {
    TEX_ROOT"/asphalt.png",TEX_ROOT"/gravel.png",TEX_ROOT"/water-lake.png",
    TEX_ROOT"/water-lake.png",TEX_ROOT"/water-lake.png",TEX_ROOT"/city1.png",
    TEX_ROOT"/drycrop1.png",TEX_ROOT"/irrcrop1.png",TEX_ROOT"/mixedcrop1.png",
    TEX_ROOT"/naturalcrop1.png",TEX_ROOT"/cropgrass1.png",TEX_ROOT"/cropgrass1.png",
    TEX_ROOT"/shrub1.png",TEX_ROOT"/deciduous1.png",TEX_ROOT"/forest1a.png",
    TEX_ROOT"/mixedforest.png",TEX_ROOT"/shrub1.png",TEX_ROOT"/airport.png",
    TEX_ROOT"/airport.png",TEX_ROOT"/rock.png",TEX_ROOT"/glacier3.png",
    TEX_ROOT"/golfcourse1.png",TEX_ROOT"/airport.png",TEX_ROOT"/deciduous1.png",
    TEX_ROOT"/city1.png",TEX_ROOT"/water-lake.png",TEX_ROOT"/rock.png",
    TEX_ROOT"/irrcrop1.png",TEX_ROOT"/asphalt.png",TEX_ROOT"/rock.png",
    TEX_ROOT"/Town1.png",TEX_ROOT"/gravel.png",TEX_ROOT"/irrcrop1.png",
    TEX_ROOT"/Runway/lf_dbl_solid_yellow.png",TEX_ROOT"/Runway/lf_runway_hold_border.png",
    TEX_ROOT"/Runway/pa_0l.png",TEX_ROOT"/Runway/pa_2l.png",TEX_ROOT"/Runway/pa_2r.png",
    TEX_ROOT"/Runway/pa_4r.png",TEX_ROOT"/Runway/pa_aim.png",TEX_ROOT"/Runway/pa_centerline.png",
    TEX_ROOT"/Runway/pa_dspl_arrows.png",TEX_ROOT"/Runway/pa_dspl_thresh.png",TEX_ROOT"/Runway/pa_rest.png",
    TEX_ROOT"/Runway/pa_shoulder_f1.png",TEX_ROOT"/Runway/pa_threshold.png",TEX_ROOT"/Runway/pc_helipad.png",
    TEX_ROOT"/Runway/pc_tiedown.png",TEX_ROOT"/Runway/grass_rwy.png"
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
    "pc_heli","pc_tiedown","grass_rwy"
};


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
            format = GL_BGR;
    }else if(img->format->BytesPerPixel == 4){
        internal_format = GL_RGBA;
        if(img->format->Rmask == 0xff)
            format = GL_RGBA;
        else
            format = GL_BGRA;
    }else{
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

    for(int idx = 0; idx < 49; idx++){
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
