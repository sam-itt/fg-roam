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

static Texture *_store[256]; /*TODO: Array->Hash or embed in meshes*/
static unsigned char _ntextures = 0;

static char *files[] = {"../../textures/asphalt.png","../../textures/gravel.png","../../textures/water-lake.png","../../textures/water-lake.png","../../textures/water-lake.png","../../textures/city1.png","../../textures/drycrop1.png","../../textures/irrcrop1.png","../../textures/mixedcrop1.png","../../textures/naturalcrop1.png","../../textures/cropgrass1.png","../../textures/cropgrass1.png","../../textures/shrub1.png","../../textures/deciduous1.png","../../textures/forest1a.png","../../textures/mixedforest.png","../../textures/shrub1.png","../../textures/airport.png","../../textures/airport.png","../../textures/rock.png","../../textures/glacier3.png","../../textures/golfcourse1.png","../../textures/airport.png","../../textures/deciduous1.png","../../textures/city1.png","../../textures/water-lake.png","../../textures/rock.png","../../textures/irrcrop1.png","../../textures/asphalt.png","../../textures/rock.png","../../textures/Town1.png","../../textures/gravel.png","../../textures/irrcrop1.png","../../textures/Runway/lf_dbl_solid_yellow.png","../../textures/Runway/lf_runway_hold_border.png","../../textures/Runway/pa_0l.png","../../textures/Runway/pa_2l.png","../../textures/Runway/pa_2r.png","../../textures/Runway/pa_4r.png","../../textures/Runway/pa_aim.png","../../textures/Runway/pa_centerline.png","../../textures/Runway/pa_dspl_arrows.png","../../textures/Runway/pa_dspl_thresh.png","../../textures/Runway/pa_rest.png","../../textures/Runway/pa_shoulder_f1.png","../../textures/Runway/pa_threshold.png","../../textures/Runway/pc_helipad.png","../../textures/Runway/pc_tiedown.png","../../textures/Runway/grass_rwy.png" };
static char *names[] = {"Freeway","Railroad","Stream","Watercourse","Canal","Urban","DryCrop","IrrCrop","ComplexCrop","NaturalCrop","CropGrass","Grassland","Scrub","DeciduousForest","EvergreenForest","MixedForest","Sclerophyllous","Airport","Grass","BarrenCover","Glacier","GolfCourse","Greenspace","Heath","Industrial","Lake","OpenMining","Orchard","Road","Rock","Town","Transport","Vineyard","lf_dbl_solid_yellow","lf_runway_hold_border","pa_0l","pa_2l","pa_2r","pa_4r","pa_aim","pa_centerline","pa_dspl_arrows","pa_dspl_thresh","pa_rest","pa_shoulder_f","pa_threshold","pc_heli","pc_tiedown","grass_rwy"};

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
