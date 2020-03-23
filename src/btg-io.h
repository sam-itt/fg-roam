#ifndef BTG_IO_H
#define BTG_IO_H
#include <stdbool.h>

#include <glib.h>
#include <zlib.h>

#define MAX_TC_SETS     (4)
#define MAX_VAS         (8)


typedef struct {
    double x;
    double y;
    double z;
} SGVec3d;

typedef struct {
    float x;
    float y;
    float z;
} SGVec3f;

#define sg_vec_equals(a, b) ((a)->x == (b)->x) && ((a)->y == (b)->y) && ((a)->z == (b)->z)

typedef struct {
    float x;
    float y;
} SGVec2f;


typedef struct {
    float red;
    float green;
    float blue;
    float alpha;
} SGVec4f;


typedef struct {
    SGVec3d center;
    double radius;
} SGSphered;


typedef struct {
    unsigned short version;

    SGVec3d gbs_center;
    float gbs_radius;

    GArray *wgs84_nodes;   // vertex list: GArray of SGVec3d
    GArray *colors;        // color list: GArray of SGVec3d
    GArray *normals;       // normal list: GArray of SGVec3f
    GArray *texcoords;     // texture coordinate list: GArray of SGVec2f
    GArray *va_flt;        // vertex attribute list (floats): GArray of float
    GArray *va_int;        // vertex attribute list (ints): GArray of int

/*
typedef std::vector < int > int_list;
typedef std::vector < int_list > group_list;

typedef std::vector < tci_list > group_tci_list;
typedef boost::array<int_list, MAX_TC_SETS> tci_list;

typedef boost::array<int_list, MAX_VAS>     vai_list;
typedef std::vector < vai_list > group_vai_list;
*/
    GPtrArray *pts_v;   // points vertex index: PtrArray of int GArray
    GPtrArray *pts_n;   // points normal index: PtrArray of int GArray
    GPtrArray *pts_c;   // points color index: PtrArray of int GArray
    GPtrArray *pts_tcs; // points texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray
    GPtrArray *pts_vas; // points vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    GPtrArray *pt_materials; // points materials: GPtrArray of char * 

    GPtrArray *tris_v;              	// triangles vertex index: PtrArray of int GArray 
    GPtrArray *tris_n;              	// triangles normal index: PtrArray of int GArray 
    GPtrArray *tris_c;              	// triangles color index: PtrArray of int GArray
    GPtrArray *tris_tcs;            // triangles texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray
    GPtrArray *tris_vas;            // triangles vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    GPtrArray *tri_materials;          // triangles materials: GPtrArray of char *

    GPtrArray *strips_v;            	// tristrips vertex index: PtrArray of int GArray
    GPtrArray *strips_n;            	// tristrips normal index: PtrArray of int GArray
    GPtrArray *strips_c;            	// tristrips color index: PtrArray of int GArray
    GPtrArray *strips_tcs;          // tristrips texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray
    GPtrArray *strips_vas;          // tristrips vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    GPtrArray *strip_materials;        // tristrips materials: GPtrArray of char *

    GPtrArray *fans_v;              	// fans vertex index: PtrArray of int GArray
    GPtrArray *fans_n;              	// fans normal index: PtrArray of int GArray
    GPtrArray *fans_c;              	// fans color index: PtrArray of int GArray
    GPtrArray *fans_tcs;            // fanss texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray 
    GPtrArray *fans_vas;            // fans vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    GPtrArray *fan_materials;	        // fans materials: GPtrArray of char *
} SGBinObject;


SGBinObject *sg_bin_object_new(void);
void sg_bin_object_free(SGBinObject *self);
bool sg_bin_object_write_obj(SGBinObject *self, const char *filename);
void sg_bin_object_load(SGBinObject *self, const char *filename);


//TODO:Move in a dedicated file
double sg_vect3d_distSqr(SGVec3d *a, SGVec3d *b);
void sg_sphered_expand_by(SGSphered *self, SGVec3d *v);
#endif
