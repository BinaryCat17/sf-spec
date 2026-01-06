#ifndef SF_TYPES_H
#define SF_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

// --- Basic Types ---
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

typedef float    f32;
typedef double   f64;

#define SF_KB(x) ((x) * 1024LL)
#define SF_MB(x) (SF_KB(x) * 1024LL)
#define SF_GB(x) (SF_MB(x) * 1024LL)

// --- Math Types ---
typedef struct { f32 x, y; } sf_vec2;
typedef struct { f32 x, y, z; } sf_vec3;
typedef struct { f32 x, y, z, w; } sf_vec4;

// Column-major 4x4 matrix
typedef struct { f32 m[16]; } sf_mat4;

// Column-major 3x3 matrix
typedef struct { f32 m[9]; } sf_mat3;

#define SF_MAX_DIMS 8
#define SF_MAX_REGISTERS 512

// --- Source Tracking ---
typedef struct {
    const char* file;
    uint32_t line;
    uint32_t column;
} sf_source_loc;

// --- Data Types ---
typedef enum {
    SF_DTYPE_UNKNOWN = 0,
    SF_DTYPE_F32,   // Standard float
    SF_DTYPE_I32,   // Integer / String ID
    SF_DTYPE_U8,    // Byte / Bool
    SF_DTYPE_COUNT
} sf_dtype;

// --- Tensor Metadata (Value Semantics) ---
// Describes the "Shape" of data, independent of storage.
typedef struct {
    sf_dtype dtype;
    uint8_t ndim; // Rank
    int32_t shape[SF_MAX_DIMS];
    int32_t strides[SF_MAX_DIMS]; // Steps in elements (not bytes) to next index
} sf_type_info;

static inline size_t sf_dtype_size(sf_dtype type) {
    switch(type) {
        case SF_DTYPE_F32: return 4;
        case SF_DTYPE_I32: return 4;
        case SF_DTYPE_U8:  return 1;
        default: return 0;
    }
}

static inline void sf_type_info_init_contiguous(sf_type_info* info, sf_dtype dtype, const int32_t* shape, uint8_t ndim) {
    info->dtype = dtype;
    info->ndim = ndim;
    if (ndim > 0 && shape) {
        for (int i = 0; i < ndim; ++i) info->shape[i] = shape[i];
        
        int32_t stride = 1;
        for (int k = ndim - 1; k >= 0; --k) {
            info->strides[k] = stride;
            stride *= (shape[k] > 0 ? shape[k] : 1);
        }
    } else {
        info->ndim = 0;
    }
}

/**
 * @brief Parses a string into an sf_dtype.
 * Case-insensitive, supports: "f32", "i32", "u8", "bool".
 */
sf_dtype sf_dtype_from_str(const char* s);

// --- Resource Flags ---
#define SF_RESOURCE_FLAG_READONLY    (1 << 0) // Cannot be bound to an Output port
#define SF_RESOURCE_FLAG_PERSISTENT  (1 << 1) // Force double-buffering (state)
#define SF_RESOURCE_FLAG_TRANSIENT   (1 << 2) // Single-buffered (scratchpad)
#define SF_RESOURCE_FLAG_SCREEN_SIZE (1 << 3) // Auto-resize with window
#define SF_RESOURCE_FLAG_OUTPUT      (1 << 4) // Primary output for display

// --- Access Modes ---
typedef enum {
    SF_ACCESS_READ = 0,
    SF_ACCESS_WRITE = 1,
    SF_ACCESS_RW = 2
} sf_access_mode;

#endif // SF_TYPES_H

