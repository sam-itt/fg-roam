#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "btg-io.h"
#include "sg-sphere.h"

#define SG_SCENERY_FILE_FORMAT "0.4"

enum sgObjectTypes {
    SG_BOUNDING_SPHERE = 0,

    SG_VERTEX_LIST = 1,
    SG_NORMAL_LIST = 2,
    SG_TEXCOORD_LIST = 3,
    SG_COLOR_LIST = 4,
    SG_VA_FLOAT_LIST = 5,
    SG_VA_INTEGER_LIST = 6,

    SG_POINTS = 9,

    SG_TRIANGLE_FACES = 10,
    SG_TRIANGLE_STRIPS = 11,
    SG_TRIANGLE_FANS = 12
};

enum sgIndexTypes {
    SG_IDX_VERTICES =    0x01,
    SG_IDX_NORMALS =     0x02,
    SG_IDX_COLORS =      0x04,
    SG_IDX_TEXCOORDS_0 = 0x08,
    SG_IDX_TEXCOORDS_1 = 0x10,
    SG_IDX_TEXCOORDS_2 = 0x20,
    SG_IDX_TEXCOORDS_3 = 0x40,
};

enum sgVertexAttributeTypes {
    // vertex attributes
    SG_VA_INTEGER_0 = 0x00000001,
    SG_VA_INTEGER_1 = 0x00000002,
    SG_VA_INTEGER_2 = 0x00000004,
    SG_VA_INTEGER_3 = 0x00000008,

    SG_VA_FLOAT_0 =   0x00000100,
    SG_VA_FLOAT_1 =   0x00000200,
    SG_VA_FLOAT_2 =   0x00000400,
    SG_VA_FLOAT_3 =   0x00000800,
};

enum sgPropertyTypes {
    SG_MATERIAL = 0,
    SG_INDEX_TYPES = 1,
    SG_VERT_ATTRIBS = 2
};


static int read_error = false;

void sgClearReadError() { read_error = false; }
int sgReadError() { return  read_error ; }

void sgReadChar ( gzFile fd, char *var )
{
    if ( gzread ( fd, var, sizeof(char) ) != sizeof(char) ) {
        read_error = true ;
    }
}


void sgReadFloat ( gzFile fd, float *var )
{
    union { float v; uint32_t u; } buf;
    if ( gzread ( fd, &buf.u, sizeof(float) ) != sizeof(float) ) {
        read_error = true ;
    }
    *var = buf.v;
}


void sgReadDouble ( gzFile fd, double *var )
{
    union { double v; uint64_t u; } buf;
    if ( gzread ( fd, &buf.u, sizeof(double) ) != sizeof(double) ) {
        read_error = true ;
    }
    *var = buf.v;
}


void sgReadUInt ( gzFile fd, unsigned int *var )
{
    if ( gzread ( fd, var, sizeof(unsigned int) ) != sizeof(unsigned int) ) {
        read_error = true ;
    }
}


void sgReadInt ( gzFile fd, int *var )
{
    if ( gzread ( fd, var, sizeof(int) ) != sizeof(int) ) {
        read_error = true ;
    }
}


void sgReadLong ( gzFile fd, int32_t *var )
{
    if ( gzread ( fd, var, sizeof(int32_t) ) != sizeof(int32_t) ) {
        read_error = true ;
    }
}


void sgReadLongLong ( gzFile fd, int64_t *var )
{
    if ( gzread ( fd, var, sizeof(int64_t) ) != sizeof(int64_t) ) {
        read_error = true ;
    }
}


void sgReadUShort ( gzFile fd, unsigned short *var )
{
    if ( gzread ( fd, var, sizeof(unsigned short) ) != sizeof(unsigned short) ){
        read_error = true ;
    }
}


void sgReadShort ( gzFile fd, short *var )
{
    if ( gzread ( fd, var, sizeof(short) ) != sizeof(short) ) {
        read_error = true ;
    }
}

void sgReadFloats ( gzFile fd, const unsigned int n, float *var )
{
    if ( gzread ( fd, var, sizeof(float) * n ) != (int)(sizeof(float) * n) ) {
        read_error = true ;
    }
}


void sgReadDoubles ( gzFile fd, const unsigned int n, double *var )
{
    if ( gzread ( fd, var, sizeof(double) * n ) != (int)(sizeof(double) * n) ) {
        read_error = true ;
    }
}


void sgReadBytes ( gzFile fd, const unsigned int n, void *var ) 
{
    if ( n == 0) return;
    if ( gzread ( fd, var, n ) != (int)n ) {
        read_error = true ;
    }
}


void sgReadUShorts ( gzFile fd, const unsigned int n, unsigned short *var )
{
    if ( gzread ( fd, var, sizeof(unsigned short) * n )
	 != (int)(sizeof(unsigned short) * n) )
    {
        read_error = true ;
    }
}


void sgReadShorts ( gzFile fd, const unsigned int n, short *var )
{
    if ( gzread ( fd, var, sizeof(short) * n )
	 != (int)(sizeof(short) * n) )
    {
        read_error = true ;
    }
}


void sgReadUInts ( gzFile fd, const unsigned int n, unsigned int *var )
{
    if ( gzread ( fd, var, sizeof(unsigned int) * n )
	 != (int)(sizeof(unsigned int) * n) )
    {
        read_error = true ;
    }
}


void sgReadInts ( gzFile fd, const unsigned int n, int *var )
{
    if ( gzread ( fd, var, sizeof(int) * n )
	 != (int)(sizeof(int) * n) )
    {
        read_error = true ;
    }
}
/***/

typedef struct{
    GByteArray *buffer;
    size_t offset;
} SGSimpleBuffer;

#define G_BYTE_ARRAY(buffer) ((GByteArray *)(buffer))

SGSimpleBuffer *sg_simple_buffer_new(void)
{
    SGSimpleBuffer *rv;

    rv = g_new0(SGSimpleBuffer, 1);
    rv->buffer = g_byte_array_new();

    return rv;
}

SGSimpleBuffer *sg_simple_buffer_sized_new(size_t size)
{
    SGSimpleBuffer *rv;

    rv = g_new0(SGSimpleBuffer, 1);
    rv->buffer = g_byte_array_sized_new(size);

    return rv;
}

void sg_simple_buffer_free(SGSimpleBuffer *self)
{
    g_byte_array_free(self->buffer, TRUE);
    g_free(self);
}

void sg_simple_buffer_resize(SGSimpleBuffer *self, size_t size)
{
    g_byte_array_set_size(self->buffer, size);
}


void sg_simple_buffer_reset(SGSimpleBuffer *self)
{
    self->offset = 0;
}

int32_t sg_simple_buffer_readInt(SGSimpleBuffer *self)
{
    unsigned int* p = (unsigned int*)(self->buffer->data + self->offset);

    self->offset += sizeof(unsigned int);
    return *p;
}

SGVec3d sg_simple_buffer_readVec3d(SGSimpleBuffer *self)
{
    double *p = (double*)(self->buffer->data + self->offset);


    self->offset += 3 * sizeof(double);
    return (*(SGVec3d *)p);
}

float sg_simple_buffer_readFloat(SGSimpleBuffer *self)
{
    float* p = (float*)(self->buffer->data + self->offset);

    self->offset += sizeof(float);
    return *p;
}

SGVec2f sg_simple_buffer_readVec2f(SGSimpleBuffer *self)
{
    float* p = (float*)(self->buffer->data + self->offset);

    self->offset += 2 * sizeof(float);
    return *((SGVec2f *)p);
}

SGVec3f sg_simple_buffer_readVec3f(SGSimpleBuffer *self)
{
    float* p = (float*)(self->buffer->data + self->offset);

    self->offset += 3 * sizeof(float);
    return *((SGVec3f*)p);
}

SGVec4f sg_simple_buffer_readVec4f(SGSimpleBuffer *self)
{
    float* p = (float*)(self->buffer->data + self->offset);

    self->offset += 4 * sizeof(float);
    return *((SGVec4f *)p);
}

/****/



gzFile file_fopen(const char *filename)
{
    gzFile rv;
    char *with_gz;

    rv = gzopen(filename, "rb");
    if (rv == NULL) {
        asprintf(&with_gz, "%s.gz", filename);
        rv = gzopen(with_gz, "rb");
        free(with_gz);
    }
    return rv;
}

SGBinObject *sg_bin_object_new(void)
{
    SGBinObject *rv;

    rv = g_new0(SGBinObject, 1);

    rv->wgs84_nodes = g_array_new(FALSE, TRUE, sizeof(SGVec3d));
    //printf("rv->wgs84_nodes: %p\n",rv->wgs84_nodes);
    rv->colors = g_array_new(FALSE, TRUE, sizeof(SGVec3d));
    rv->normals = g_array_new(FALSE, TRUE, sizeof(SGVec3f));
    rv->texcoords = g_array_new(FALSE, TRUE, sizeof(SGVec2f));
    rv->va_flt = g_array_new(FALSE, TRUE, sizeof(float));
    rv->va_int = g_array_new(FALSE, TRUE, sizeof(int));

    rv->pts_v = g_ptr_array_new();
    rv->pts_n = g_ptr_array_new();
    rv->pts_c = g_ptr_array_new();
    rv->pts_tcs = g_ptr_array_new();
    rv->pts_vas = g_ptr_array_new();
    rv->pt_materials = g_ptr_array_new();

    rv->tris_v = g_ptr_array_new();
    rv->tris_n = g_ptr_array_new();
    rv->tris_c = g_ptr_array_new();
    rv->tris_tcs = g_ptr_array_new();
    rv->tris_vas = g_ptr_array_new();
    rv->tri_materials = g_ptr_array_new();

    rv->strips_v = g_ptr_array_new();
    rv->strips_n = g_ptr_array_new();
    rv->strips_c = g_ptr_array_new();
    rv->strips_tcs = g_ptr_array_new();
    rv->strips_vas = g_ptr_array_new();
    rv->strip_materials = g_ptr_array_new();

    rv->fans_v = g_ptr_array_new();
    rv->fans_n = g_ptr_array_new();
    rv->fans_c = g_ptr_array_new();
    rv->fans_tcs = g_ptr_array_new();
    rv->fans_vas = g_ptr_array_new();
    rv->fan_materials = g_ptr_array_new();

    return rv;
}

void sg_bin_object_free(SGBinObject *self)
{

    g_array_free(self->wgs84_nodes, TRUE);
    g_array_free(self->colors, TRUE);
    g_array_free(self->normals, TRUE);
    g_array_free(self->texcoords, TRUE);
    g_array_free(self->va_flt, TRUE);
    g_array_free(self->va_int, TRUE);

    /*Points*/
    // points vertex index: PtrArray of int GArray
    for(guint i = 0; i < self->pts_v->len; i++){
        GArray *a = g_ptr_array_index(self->pts_v, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->pts_v, TRUE);

    // points normal index: PtrArray of int GArray
    for(guint i = 0; i < self->pts_n->len; i++){
        GArray *a = g_ptr_array_index(self->pts_n, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->pts_n, TRUE);

    // points color index: PtrArray of int GArray
    for(guint i = 0; i < self->pts_c->len; i++){
        GArray *a = g_ptr_array_index(self->pts_c, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->pts_c, TRUE);

    // points texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->pts_tcs->len; i++){
        GPtrArray *p_tcs = g_ptr_array_index(self->pts_tcs, i);
        for(guint j = 0; j < p_tcs->len; j++){
            GArray *a = g_ptr_array_index(p_tcs, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(p_tcs, TRUE);
    }
    g_ptr_array_free(self->pts_tcs, TRUE);

    // points vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->pts_vas->len; i++){
        GPtrArray *p_vas = g_ptr_array_index(self->pts_vas, i);
        for(guint j = 0; j < p_vas->len; j++){
            GArray *a = g_ptr_array_index(p_vas, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(p_vas, TRUE);
    }
    g_ptr_array_free(self->pts_vas, TRUE);

    // points materials: GPtrArray of char *
    for(guint i = 0; i < self->pt_materials->len; i++){
        char *str = g_ptr_array_index(self->pt_materials, i);
        g_free(str);
    }
    g_ptr_array_free(self->pt_materials, TRUE);

    /*Triangles*/
    // triangles vertex index: PtrArray of int GArray
    for(guint i = 0; i < self->tris_v->len; i++){
        GArray *a = g_ptr_array_index(self->tris_v, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->tris_v, TRUE);

    // triangles normal index: PtrArray of int GArray
    for(guint i = 0; i < self->tris_n->len; i++){
        GArray *a = g_ptr_array_index(self->tris_n, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->tris_n, TRUE);

    // triangles color index: PtrArray of int GArray
    for(guint i = 0; i < self->tris_c->len; i++){
        GArray *a = g_ptr_array_index(self->tris_c, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->tris_c, TRUE);

    // triangles texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->tris_tcs->len; i++){
        GPtrArray *t_tcs = g_ptr_array_index(self->tris_tcs, i);
        for(guint j = 0; j < t_tcs->len; j++){
            GArray *a = g_ptr_array_index(t_tcs, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(t_tcs, TRUE);
    }
    g_ptr_array_free(self->tris_tcs, TRUE);

    // triangles vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->tris_vas->len; i++){
        GPtrArray *t_vas = g_ptr_array_index(self->tris_vas, i);
        for(guint j = 0; j < t_vas->len; j++){
            GArray *a = g_ptr_array_index(t_vas, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(t_vas, TRUE);
    }
    g_ptr_array_free(self->tris_vas, TRUE);

    // triangles materials: GPtrArray of char *
    for(guint i = 0; i < self->tri_materials->len; i++){
        char *str = g_ptr_array_index(self->tri_materials, i);
        g_free(str);
    }
    g_ptr_array_free(self->tri_materials, TRUE);

    /*Triangle strips*/
     // strips vertex index: PtrArray of int GArray
    for(guint i = 0; i < self->strips_v->len; i++){
        GArray *a = g_ptr_array_index(self->strips_v, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->strips_v, TRUE);

    // strips normal index: PtrArray of int GArray
    for(guint i = 0; i < self->strips_n->len; i++){
        GArray *a = g_ptr_array_index(self->strips_n, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->strips_n, TRUE);

    // strips color index: PtrArray of int GArray
    for(guint i = 0; i < self->strips_c->len; i++){
        GArray *a = g_ptr_array_index(self->strips_c, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->strips_c, TRUE);

    // strips texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->strips_tcs->len; i++){
        GPtrArray *s_tcs = g_ptr_array_index(self->strips_tcs, i);
        for(guint j = 0; j < s_tcs->len; j++){
            GArray *a = g_ptr_array_index(s_tcs, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(s_tcs, TRUE);
    }
    g_ptr_array_free(self->strips_tcs, TRUE);

    // strips vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->strips_vas->len; i++){
        GPtrArray *s_vas = g_ptr_array_index(self->strips_vas, i);
        for(guint j = 0; j < s_vas->len; j++){
            GArray *a = g_ptr_array_index(s_vas, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(s_vas, TRUE);
    }
    g_ptr_array_free(self->strips_vas, TRUE);

    // strips materials: GPtrArray of char *
    for(guint i = 0; i < self->strip_materials->len; i++){
        char *str = g_ptr_array_index(self->strip_materials, i);
        g_free(str);
    }
    g_ptr_array_free(self->strip_materials, TRUE);

    /*Triangle fans*/
     // fans vertex index: PtrArray of int GArray
    for(guint i = 0; i < self->fans_v->len; i++){
        GArray *a = g_ptr_array_index(self->fans_v, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->fans_v, TRUE);

    // fans normal index: PtrArray of int GArray
    for(guint i = 0; i < self->fans_n->len; i++){
        GArray *a = g_ptr_array_index(self->fans_n, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->fans_n, TRUE);

    // fans color index: PtrArray of int GArray
    for(guint i = 0; i < self->fans_c->len; i++){
        GArray *a = g_ptr_array_index(self->fans_c, i);
        g_array_free(a, TRUE);
    }
    g_ptr_array_free(self->fans_c, TRUE);

    // fans texture coordinates ( up to 4 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->fans_tcs->len; i++){
        GPtrArray *f_tcs = g_ptr_array_index(self->fans_tcs, i);
        for(guint j = 0; j < f_tcs->len; j++){
            GArray *a = g_ptr_array_index(f_tcs, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(f_tcs, TRUE);
    }
    g_ptr_array_free(self->fans_tcs, TRUE);

    // fans vertex attributes ( up to 8 sets ): PtrArray of sized PtrArray of int GArray
    for(guint i = 0; i < self->fans_vas->len; i++){
        GPtrArray *f_vas = g_ptr_array_index(self->fans_vas, i);
        for(guint j = 0; j < f_vas->len; j++){
            GArray *a = g_ptr_array_index(f_vas, j);
            g_array_free(a, TRUE);
        }
        g_ptr_array_free(f_vas, TRUE);
    }
    g_ptr_array_free(self->fans_vas, TRUE);

    // fans materials: GPtrArray of char *
    for(guint i = 0; i < self->fan_materials->len; i++){
        char *str = g_ptr_array_index(self->fan_materials, i);
        g_free(str);
    }
    g_ptr_array_free(self->fan_materials, TRUE);

    g_free(self);
}



void sg_bin_object_read_properties(gzFile fp, int nproperties)
{
    SGSimpleBuffer *buf;
    uint32_t nbytes;

    // read properties
    buf = sg_simple_buffer_new();
    for ( int j = 0; j < nproperties; ++j ) {
        char prop_type;
        sgReadChar( fp, &prop_type );
        sgReadUInt( fp, &nbytes );
//        printf("property size = %d\n", nbytes);
        if ( nbytes > buf->buffer->len) { sg_simple_buffer_resize(buf, nbytes ); }
        char *ptr = buf->buffer->data;
        sgReadBytes( fp, nbytes, ptr );
    }
    sg_simple_buffer_free(buf);
}

static void read_indices_uint32(char* buffer,
                         size_t bytes,
                         int indexMask,
                         int vaMask,
                         GArray *vertices,
                         GArray *normals,
                         GArray *colors,
                         GPtrArray *texCoords,
                         GPtrArray *vas
                        )
{
    const int indexSize = sizeof(uint32_t) * __builtin_popcount(indexMask);
    const int vaSize = sizeof(uint32_t) * __builtin_popcount(vaMask);
    const int count = bytes / (indexSize + vaSize);

    uint32_t *src = (uint32_t *)(buffer);
    for (int i=0; i<count; ++i) {
        if (indexMask & SG_IDX_VERTICES)  g_array_append_val(vertices, *src++);
        if (indexMask & SG_IDX_NORMALS) g_array_append_val(normals, *src++);
        if (indexMask & SG_IDX_COLORS) g_array_append_val(colors, *src++);
        if (indexMask & SG_IDX_TEXCOORDS_0) g_array_append_val(g_ptr_array_index(texCoords, 0), *src++);
        if (indexMask & SG_IDX_TEXCOORDS_1) g_array_append_val(g_ptr_array_index(texCoords,1), *src++);
        if (indexMask & SG_IDX_TEXCOORDS_2) g_array_append_val(g_ptr_array_index(texCoords,2), *src++);
        if (indexMask & SG_IDX_TEXCOORDS_3) g_array_append_val(g_ptr_array_index(texCoords,3), *src++);

        if ( vaMask ) {
            if (vaMask & SG_VA_INTEGER_0) g_array_append_val(g_ptr_array_index(vas,0), *src++);
            if (vaMask & SG_VA_INTEGER_1) g_array_append_val(g_ptr_array_index(vas,1), *src++);
            if (vaMask & SG_VA_INTEGER_2) g_array_append_val(g_ptr_array_index(vas,2), *src++);
            if (vaMask & SG_VA_INTEGER_3) g_array_append_val(g_ptr_array_index(vas,3), *src++);
            if (vaMask & SG_VA_FLOAT_0) g_array_append_val(g_ptr_array_index(vas,4), *src++);
            if (vaMask & SG_VA_FLOAT_1) g_array_append_val(g_ptr_array_index(vas,5), *src++);
            if (vaMask & SG_VA_FLOAT_2) g_array_append_val(g_ptr_array_index(vas,6), *src++);
            if (vaMask & SG_VA_FLOAT_3) g_array_append_val(g_ptr_array_index(vas,7), *src++);
        }
    } // of elements in the index

    // WS2.0 fix : toss zero area triangles
    if ( ( count == 3 ) && (indexMask & SG_IDX_VERTICES) ) {
        if ( (g_array_index(vertices,uint32_t,0) == g_array_index(vertices,uint32_t,1)) ||
             (g_array_index(vertices,uint32_t,1) == g_array_index(vertices,uint32_t,2)) ||
             (g_array_index(vertices,uint32_t,2) == g_array_index(vertices,uint32_t,0)) ) {
            g_array_remove_range(vertices, 0, vertices->len);
        }
    }
}

static void read_indices_uint16(char* buffer,
                         size_t bytes,
                         int indexMask,
                         int vaMask,
                         GArray *vertices,
                         GArray *normals,
                         GArray *colors,
                         GPtrArray *texCoords,
                         GPtrArray *vas
                        )
{

    const int indexSize = sizeof(uint16_t) * __builtin_popcount(indexMask);
    const int vaSize = sizeof(uint16_t) * __builtin_popcount(vaMask);
    const int count = bytes / (indexSize + vaSize);

    uint16_t *src = (uint16_t *)(buffer);
    for (int i=0; i<count; ++i) {
        if (indexMask & SG_IDX_VERTICES)  g_array_append_val(vertices, *src++);
        if (indexMask & SG_IDX_NORMALS) g_array_append_val(normals, *src++);
        if (indexMask & SG_IDX_COLORS) g_array_append_val(colors, *src++);
        if (indexMask & SG_IDX_TEXCOORDS_0) g_array_append_val(g_ptr_array_index(texCoords,0), *src++);
        if (indexMask & SG_IDX_TEXCOORDS_1) g_array_append_val(g_ptr_array_index(texCoords,1), *src++);
        if (indexMask & SG_IDX_TEXCOORDS_2) g_array_append_val(g_ptr_array_index(texCoords,2), *src++);
        if (indexMask & SG_IDX_TEXCOORDS_3) g_array_append_val(g_ptr_array_index(texCoords,3), *src++);

        if ( vaMask ) {
            if (vaMask & SG_VA_INTEGER_0) g_array_append_val(g_ptr_array_index(vas,0), *src++);
            if (vaMask & SG_VA_INTEGER_1) g_array_append_val(g_ptr_array_index(vas,1), *src++);
            if (vaMask & SG_VA_INTEGER_2) g_array_append_val(g_ptr_array_index(vas,2), *src++);
            if (vaMask & SG_VA_INTEGER_3) g_array_append_val(g_ptr_array_index(vas,3), *src++);
            if (vaMask & SG_VA_FLOAT_0) g_array_append_val(g_ptr_array_index(vas,4), *src++);
            if (vaMask & SG_VA_FLOAT_1) g_array_append_val(g_ptr_array_index(vas,5), *src++);
            if (vaMask & SG_VA_FLOAT_2) g_array_append_val(g_ptr_array_index(vas,6), *src++);
            if (vaMask & SG_VA_FLOAT_3) g_array_append_val(g_ptr_array_index(vas,7), *src++);
        }
    } // of elements in the index

    // WS2.0 fix : toss zero area triangles
    if ( ( count == 3 ) && (indexMask & SG_IDX_VERTICES) ) {
        if ( (g_array_index(vertices,uint16_t,0) == g_array_index(vertices,uint16_t,1)) ||
             (g_array_index(vertices,uint16_t,1) == g_array_index(vertices,uint16_t,2)) ||
             (g_array_index(vertices,uint16_t,2) == g_array_index(vertices,uint16_t,0)) ) {
            g_array_remove_range(vertices, 0, vertices->len);
        }
    }
}


// read object properties
void sg_bin_object_read_object(SGBinObject *self, gzFile fp,
                         int obj_type,
                         int nproperties,
                         int nelements,
                         GPtrArray *vertices,
                         GPtrArray *normals,
                         GPtrArray *colors,
                         GPtrArray *texCoords,
                         GPtrArray *vertexAttribs,
                         GPtrArray *materials)
{
    unsigned int  nbytes;
    unsigned char idx_mask;
    unsigned int  vertex_attrib_mask;
    int j;
    GArray *vs; 
    GArray *ns; 
    GArray *cs; 
    GPtrArray *tcs; 
    GPtrArray *vas; 
    char material[256];
    SGSimpleBuffer *buf;

    buf = sg_simple_buffer_sized_new(32768); //32 kb

    // default values
    if ( obj_type == SG_POINTS ) {
        idx_mask = SG_IDX_VERTICES;
    } else {
        idx_mask = (char)(SG_IDX_VERTICES | SG_IDX_TEXCOORDS_0);
    }
    vertex_attrib_mask = 0;

    for ( j = 0; j < nproperties; ++j ) {
        char prop_type;
        sgReadChar( fp, &prop_type );
        sgReadUInt( fp, &nbytes );

        sg_simple_buffer_resize(buf, nbytes);
        char *ptr = buf->buffer->data;

        switch( prop_type )
        {
            case SG_MATERIAL:
                sgReadBytes( fp, nbytes, ptr );
                if (nbytes > 255) {
                    nbytes = 255;
                }
                strncpy( material, ptr, nbytes );
                material[nbytes] = '\0';
                break;

            case SG_INDEX_TYPES:
                if (nbytes == 1) {
                    sgReadChar( fp, (char *)&idx_mask );
                } else {
                    sgReadBytes( fp, nbytes, ptr );
                }
                break;

            case SG_VERT_ATTRIBS:
                if (nbytes == 4) {
                    sgReadUInt( fp, &vertex_attrib_mask );
                } else {
                    sgReadBytes( fp, nbytes, ptr );
                }
                break;

            default:
                sgReadBytes( fp, nbytes, ptr );
                printf("Found UNKNOWN property type with nbytes == %d mask is %d\n", nbytes, (int)idx_mask);
                break;
        }
    }

    if ( sgReadError() ) {
        printf("Error reading object properties\n");
    }

    size_t indexCount = __builtin_popcount(idx_mask);
    if (indexCount == 0) {
        printf("object index mask has no bits set\n");
    }

    for ( j = 0; j < nelements; ++j ) {
        sgReadUInt( fp, &nbytes );
        if ( sgReadError() ) {
                printf("Error reading element size");
        }

        sg_simple_buffer_resize(buf, nbytes);
        char *ptr = buf->buffer->data;
        sgReadBytes( fp, nbytes, ptr );

        if ( sgReadError() ) {
            printf("Error reading element bytes");
        }

        size_t indices_size;
        indices_size = (self->version >= 10) ? sizeof(uint32_t) : sizeof(uint16_t);
        vs = g_array_new(FALSE, TRUE, indices_size);
        ns = g_array_new(FALSE, TRUE, indices_size);
        cs = g_array_new(FALSE, TRUE, indices_size);
        tcs = g_ptr_array_sized_new(MAX_TC_SETS);
        for(int z = 0; z < MAX_TC_SETS; z++)
            g_ptr_array_add(tcs, g_array_new(FALSE, TRUE, indices_size));
        vas = g_ptr_array_sized_new(MAX_VAS);
        for(int z = 0; z < MAX_VAS; z++)
            g_ptr_array_add(vas, g_array_new(FALSE, TRUE, indices_size));

        if (self->version >= 10) {
//            printf("Reading indices as 32-bits ints\n");
            read_indices_uint32(ptr, nbytes, idx_mask, vertex_attrib_mask, vs, ns, cs, tcs, vas);
        } else {
//            printf("Reading indices as 16-bits ints\n");
            read_indices_uint16(ptr, nbytes, idx_mask, vertex_attrib_mask, vs, ns, cs, tcs, vas);
        }

        // Fix for WS2.0 - ignore zero area triangles
        if ( vs->len > 0 ) {
            g_ptr_array_add(vertices, vs);
            g_ptr_array_add(normals, ns);
            g_ptr_array_add(colors, cs);
            g_ptr_array_add(texCoords, tcs);
            g_ptr_array_add(vertexAttribs, vas);
            g_ptr_array_add(materials, g_strdup(material));
        }else{
            g_array_free(vs, TRUE);
            g_array_free(ns, TRUE);
            g_array_free(cs, TRUE);
            for(int z = 0; z < MAX_TC_SETS; z++){
                GArray *a = g_ptr_array_index(tcs, z);
                g_array_free(a, TRUE);
            }
            g_ptr_array_free(tcs, TRUE);
            for(int z = 0; z < MAX_VAS; z++){
                GArray *a = g_ptr_array_index(vas, z);
                g_array_free(a, TRUE);
            }
            g_ptr_array_free(vas, TRUE);
        }
    } // of element iteration
    sg_simple_buffer_free(buf);
}



void sg_bin_object_load(SGBinObject *self, const char *filename) 
{
    SGVec3d p;
    int i, k;
    size_t j;
    gzFile fp;
    unsigned int nbytes;
    SGSimpleBuffer *buf;

    fp = file_fopen(filename);
    if(!fp){
        printf("Error opening for reading (and .gz): %s\n",filename);
        return;
    }
   
    buf =  sg_simple_buffer_sized_new(32768); //32 kb


    sgClearReadError();

    // read headers
    unsigned int header;
    sgReadUInt( fp, &header );
    if ( ((header & 0xFF000000) >> 24) == 'S' &&
         ((header & 0x00FF0000) >> 16) == 'G' ) {

        // read file version
        self->version = (header & 0x0000FFFF);
    } else {
        // close the file before we return
        gzclose(fp);
        printf("Bad BTG magic/version\n");
        return;
    }

    // read creation time
    unsigned int foo_calendar_time;
    sgReadUInt( fp, &foo_calendar_time );

    // read number of top level objects
    int nobjects;
    if ( self->version >= 10) { // version 10 extends everything to be 32-bit
        sgReadInt( fp, &nobjects );
    } else if ( self->version >= 7 ) {
        uint16_t v;
        sgReadUShort( fp, &v );
        nobjects = v;
    } else {
        int16_t v;
        sgReadShort( fp, &v );
        nobjects = v;
    }

    //printf("SGBinObject::read_bin Total objects to read = %d\n", nobjects);

    if ( sgReadError() ) {
        printf("Error reading BTG file header\n");
        return;
    }

    // read in objects
    for ( i = 0; i < nobjects; ++i ) {
        // read object header
        char obj_type;
        uint32_t nproperties, nelements;
        sgReadChar( fp, &obj_type );
        if ( self->version >= 10 ) {
            sgReadUInt( fp, &nproperties );
            sgReadUInt( fp, &nelements );
        } else if ( self->version >= 7 ) {
            uint16_t v;
            sgReadUShort( fp, &v );
            nproperties = v;
            sgReadUShort( fp, &v );
            nelements = v;
        } else {
            int16_t v;
            sgReadShort( fp, &v );
            nproperties = v;
            sgReadShort( fp, &v );
            nelements = v;
        }

        //printf("SGBinObject::read_bin object #%d = %d props = %d elements = %d\n", i, (int)obj_type, nproperties, nelements);

        if ( obj_type == SG_BOUNDING_SPHERE ) {
            // read bounding sphere properties
            sg_bin_object_read_properties( fp, nproperties );

            // read bounding sphere elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                sg_simple_buffer_resize(buf, nbytes);
                sg_simple_buffer_reset(buf);
                char *ptr = buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
                self->gbs_center = sg_simple_buffer_readVec3d(buf);
                self->gbs_radius = sg_simple_buffer_readFloat(buf);
            }
        } else if ( obj_type == SG_VERTEX_LIST ) {
            // read vertex list properties
            sg_bin_object_read_properties( fp, nproperties );

            // read vertex list elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                sg_simple_buffer_resize(buf, nbytes);
                sg_simple_buffer_reset(buf);
                char *ptr = buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
                int count = nbytes / (sizeof(float) * 3);
   //             g_array_set_size(self->wgs84_nodes, count);
                for ( k = 0; k < count; ++k ) {
                    SGVec3f v = sg_simple_buffer_readVec3f(buf);
                    // extend from float to double, hmmm
                    SGVec3d tmp = {v.x, v.y, v.z};
                    g_array_append_val(self->wgs84_nodes, tmp);
                }
            }
        } else if ( obj_type == SG_COLOR_LIST ) {
            // read color list properties
            sg_bin_object_read_properties( fp, nproperties );

            // read color list elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                sg_simple_buffer_resize(buf, nbytes);
                sg_simple_buffer_reset(buf);
                char *ptr = buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
                int count = nbytes / (sizeof(float) * 4);
//                g_array_set_size(self->colors, count);
                for ( k = 0; k < count; ++k ) {
                    SGVec4f tmp = sg_simple_buffer_readVec4f(buf);
                    g_array_append_val(self->colors, tmp);
                }
            }
        } else if ( obj_type == SG_NORMAL_LIST ) {
            // read normal list properties
            sg_bin_object_read_properties( fp, nproperties );

            // read normal list elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                sg_simple_buffer_resize(buf, nbytes);
                sg_simple_buffer_reset(buf);
                unsigned char *ptr = (unsigned char *)buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
                int count = nbytes / 3;
//                g_array_set_size(self->normals, count);
                for ( k = 0; k < count; ++k ) {
                    SGVec3f normal = { (ptr[0]) / 127.5 - 1.0,
                                       (ptr[1]) / 127.5 - 1.0,
                                       (ptr[2]) / 127.5 - 1.0};
                    sg_vect3f_normalize(&normal);
                    g_array_append_val(self->normals, normal);
                    ptr += 3;
                }
            }
        } else if ( obj_type == SG_TEXCOORD_LIST ) {
            // read texcoord list properties
            sg_bin_object_read_properties( fp, nproperties );

            // read texcoord list elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                sg_simple_buffer_resize(buf, nbytes);
                sg_simple_buffer_reset(buf);
                char *ptr = buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
                int count = nbytes / (sizeof(float) * 2);
//                g_array_set_size(self->texcoords, count);
                for ( k = 0; k < count; ++k ) {
                    SGVec2f tmp = sg_simple_buffer_readVec2f(buf);
                    g_array_append_val(self->texcoords, tmp);
                }
            }
        } else if ( obj_type == SG_VA_FLOAT_LIST ) {
            // read vertex attribute (float) properties
            sg_bin_object_read_properties( fp, nproperties );

            // read vertex attribute list elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                sg_simple_buffer_resize(buf, nbytes);
                sg_simple_buffer_reset(buf);
                char *ptr = buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
                int count = nbytes / (sizeof(float));
//                g_array_set_size(self->va_flt, count);
                for ( k = 0; k < count; ++k ) {
                    float tmp = sg_simple_buffer_readFloat(buf);
                    g_array_append_val(self->va_flt, tmp);
                }
            }
        } else if ( obj_type == SG_VA_INTEGER_LIST ) {
            // read vertex attribute (integer) properties
            sg_bin_object_read_properties( fp, nproperties );

            // read vertex attribute list elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                sg_simple_buffer_resize(buf, nbytes);
                sg_simple_buffer_reset(buf);
                char *ptr = buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
                int count = nbytes / (sizeof(unsigned int));
//                g_array_set_size(self->va_int, count);
                for ( k = 0; k < count; ++k ) {
                    int tmp = sg_simple_buffer_readInt(buf);
                    g_array_append_val(self->va_int, tmp);
                }
            }
        } else if ( obj_type == SG_POINTS ) {
            // read point elements
            sg_bin_object_read_object(self, fp, SG_POINTS, nproperties, nelements,
                         self->pts_v, self->pts_n, self->pts_c, self->pts_tcs,
                         self->pts_vas, self->pt_materials );
        } else if ( obj_type == SG_TRIANGLE_FACES ) {
            // read triangle face properties
            sg_bin_object_read_object(self, fp, SG_TRIANGLE_FACES, nproperties, nelements,
                         self->tris_v, self->tris_n, self->tris_c, self->tris_tcs,
                         self->tris_vas, self->tri_materials );
        } else if ( obj_type == SG_TRIANGLE_STRIPS ) {
            // read triangle strip properties
            sg_bin_object_read_object(self, fp, SG_TRIANGLE_STRIPS, nproperties, nelements,
                         self->strips_v, self->strips_n, self->strips_c, self->strips_tcs,
                         self->strips_vas, self->strip_materials );
        } else if ( obj_type == SG_TRIANGLE_FANS ) {
            // read triangle fan properties
            sg_bin_object_read_object(self, fp, SG_TRIANGLE_FANS, nproperties, nelements,
                         self->fans_v, self->fans_n, self->fans_c, self->fans_tcs,
                         self->fans_vas, self->fan_materials );
        } else {
            // unknown object type, just skip
            sg_bin_object_read_properties( fp, nproperties );

            // read elements
            for ( j = 0; j < nelements; ++j ) {
                sgReadUInt( fp, &nbytes );
                // cout << "element size = " << nbytes << endl;
                if ( nbytes > buf->buffer->len ) { sg_simple_buffer_resize(buf, nbytes); }
                char *ptr = buf->buffer->data;
                sgReadBytes( fp, nbytes, ptr );
            }
        }

        if ( sgReadError() ) {
            printf("Error while reading object %d\n",i);
        }
    }

    // close the file
    gzclose(fp);
    sg_simple_buffer_free(buf);
}


bool sg_bin_object_write_obj(SGBinObject *self, const char *filename)
{
    size_t i, j;
    FILE *fp;

    fp = fopen( filename, "w" );
    if ( fp == NULL ) {
        printf("ERROR: opening %s for writing!\n", filename);
        return false;
    }

    printf("triangles size = %d tri_materials = %d\n", self->tris_v->len, self->tri_materials->len);
    printf("strips size = %d strip_materials = %d\n", self->strips_v->len, self->strip_materials->len);
    printf("fan size = %d fan_materials = %d\n", self->fans_v->len, self->fan_materials->len);

    printf("points = %d\n",self->wgs84_nodes->len);
    printf("tex coords = %d\n",self->texcoords->len);

    // write headers
    fprintf(fp, "# FGFS Scenery\n");
    fprintf(fp, "# Version %s\n", SG_SCENERY_FILE_FORMAT);

    time_t calendar_time = time(NULL);
    struct tm *local_tm;
    local_tm = localtime( &calendar_time );
    char time_str[256];
    strftime( time_str, 256, "%a %b %d %H:%M:%S %Z %Y", local_tm);
    fprintf(fp, "# Created %s\n", time_str );
    fprintf(fp, "\n");

    // write bounding sphere
    fprintf(fp, "# gbs %.5f %.5f %.5f %.2f\n",
            self->gbs_center.x, self->gbs_center.y, self->gbs_center.z, self->gbs_radius);
    fprintf(fp, "\n");

    // dump vertex list
//    fprintf(fp, "# vertex list (%d elements)\n", self->wgs84_nodes->len);
    fprintf(fp, "# vertex list\n");
    for ( i = 0; i < self->wgs84_nodes->len; ++i ) {
        SGVec3d a = g_array_index(self->wgs84_nodes, SGVec3d , i);
#if 1
        SGVec3d p = (SGVec3d){ a.x - self->gbs_center.x,
                               a.y - self->gbs_center.y,
                               a.z - self->gbs_center.z};

        fprintf(fp,  "v %.5f %.5f %.5f\n", p.x, p.y, p.z );
#elif 0
        SGVec3d p = (SGVec3d){ a.x + self->gbs_center.x,
                               a.y + self->gbs_center.y,
                               a.z + self->gbs_center.z};

        fprintf(fp,  "v %.5f %.5f %.5f\n", p.x, p.y, p.z );
#else
        fprintf(fp,  "v %.5f %.5f %.5f\n", a.x, a.y, a.z );
#endif
    }
    fprintf(fp, "\n");

    fprintf(fp, "# vertex normal list\n");
    for ( i = 0; i < self->normals->len; ++i ) {
        SGVec3f p = g_array_index(self->normals, SGVec3f , i);
        fprintf(fp,  "vn %.5f %.5f %.5f\n", p.x, p.y, p.z );
    }
    fprintf(fp, "\n");

    // dump texture coordinates
    fprintf(fp, "# texture coordinate list\n");
    for ( i = 0; i < self->texcoords->len; ++i ) {
        SGVec2f p = g_array_index(self->texcoords, SGVec2f , i);
        fprintf(fp,  "vt %.5f %.5f\n", p.x, p.y );
    }
    fprintf(fp, "\n");

    // dump individual triangles if they exist
    if ( self->tris_v->len != 0 ) {
        fprintf(fp, "# triangle groups\n");

        guint start = 0;
        guint end = 1;
        char *material;
        while ( start < self->tri_materials->len ) {
            // find next group
            material = g_ptr_array_index(self->tri_materials,start); 
           // printf("tri_materials.size: %d\n", self->tri_materials->len);
            while ( (end < self->tri_materials->len) &&
                    (!strcmp(material, g_ptr_array_index(self->tri_materials,end))) )
            {
                //printf("end = %d\n",end);
                end++;
            }
            //printf("group = %d to %d\n",start, end-1);
            //
            SGSphered d = (SGSphered){ 
                .center = (SGVec3d){0.0, 0.0, 0.0},
                .radius = -1.0 
            };
            for ( i = start; i < end; ++i ) {
                GArray *a = g_ptr_array_index(self->tris_v, i);
                for ( j = 0; j < a->len; ++j ) {
                    int idx;
                    if(self->version >= 10)
                        idx = g_array_index(a, int, j);
                    else
                        idx = g_array_index(a, uint16_t, j);
                    SGVec3d tmp = g_array_index(self->wgs84_nodes, SGVec3d, idx);
                    sg_sphered_expand_by(&d, &tmp);
                }
            }

            SGVec3d bs_center = d.center;
            double bs_radius = d.radius;

            // write group headers
            fprintf(fp, "\n");
            fprintf(fp, "# usemtl %s\n", material);
            fprintf(fp, "# bs %.4f %.4f %.4f %.2f\n",
                    bs_center.x, bs_center.y, bs_center.z, bs_radius);

            // write groups
            for ( i = start; i < end; ++i ) {
                fprintf(fp, "f");
                GArray *tri_v = g_ptr_array_index(self->tris_v, i);
                GPtrArray *tri_tcs = g_ptr_array_index(self->tris_tcs,i);
                for ( j = 0; j < tri_v->len; ++j ) {
                    int a, b;
                    GArray *ttcs = g_ptr_array_index(tri_tcs,0);
                    if(self->version >= 10){
                        a = g_array_index(tri_v, int, j);
                        b = g_array_index(ttcs, int, j);
                    }else{
                        a = g_array_index(tri_v, uint16_t, j);
                        b = g_array_index(ttcs, uint16_t, j);
                    }
                    fprintf(fp, " %d/%d", a+1, b+1);
                }
                fprintf(fp, "\n");
            }

            start = end;
            end = start + 1;
        }
    }

    // dump triangle groups
    if (self->strips_v->len != 0) {
        fprintf(fp, "# triangle strips\n");

        guint start = 0;
        guint end = 1;
        char *material;
        while ( start < self->strip_materials->len ) {
            // find next group
            material = g_ptr_array_index(self->strip_materials, start); 
            while ( (end < self->strip_materials->len) &&
                    (!strcmp(material, g_ptr_array_index(self->strip_materials, end))) )
                {
                    // cout << "end = " << end << endl;
                    end++;
                }
            // cout << "group = " << start << " to " << end - 1 << endl;
            
            
            SGSphered d = (SGSphered){ 
                .center = (SGVec3d){0.0, 0.0, 0.0},
                .radius = -1.0 
            };
            for ( i = start; i < end; ++i ) {
                GArray *a = g_ptr_array_index(self->tris_v, i);
                for ( j = 0; j < a->len; ++j ) {
                    int idx;
                    if(self->version >= 10)
                        idx = g_array_index(a, int, j);
                    else
                        idx = g_array_index(a, uint16_t, j);
                    SGVec3d tmp = g_array_index(self->wgs84_nodes, SGVec3d, idx);
                    sg_sphered_expand_by(&d, &tmp);
                }
            }

            SGVec3d bs_center = d.center;
            double bs_radius = d.radius;

            // write group headers
            fprintf(fp, "\n");
            fprintf(fp, "# usemtl %s\n", material);
            fprintf(fp, "# bs %.4f %.4f %.4f %.2f\n",
                    bs_center.x, bs_center.y, bs_center.z, bs_radius);

            // write groups
            for ( i = start; i < end; ++i ) {
                fprintf(fp, "ts");
                GArray *strip_v = g_ptr_array_index(self->strips_v, i);
                GPtrArray *strip_tcs = g_ptr_array_index(self->strips_tcs,i);
                for ( j = 0; j < strip_v->len; ++j ) {
                    int a, b;
                    GArray *stcs = g_ptr_array_index(strip_tcs,0);
                    if(self->version >= 10){
                        a = g_array_index(strip_v, int, j);
                        b = g_array_index(stcs, int, j);
                    }else{
                        a = g_array_index(strip_v, uint16_t, j);
                        b = g_array_index(stcs, uint16_t, j);
                    }
                    fprintf(fp, " %d/%d", a+1, b+1);
                }
                fprintf(fp, "\n");
            }

            start = end;
            end = start + 1;
        }
    }

    // close the file
    fclose(fp);

    return true;
}
