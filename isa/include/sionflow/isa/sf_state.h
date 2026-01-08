#ifndef SF_STATE_H
#define SF_STATE_H

#include <sionflow/base/sf_memory.h>
#include <sionflow/base/sf_platform.h>

/**
 * @brief Persistent container for tensor data and memory management.
 * Owned by the Engine. Backends read from/write to this state.
 */
typedef struct sf_state {
    void**         reg_data;        // Array of pointers to buffer data [register_count]
    uint8_t*       reg_ndims;       // Rank for each register [register_count]
    uint8_t*       reg_dtypes;      // DType for each register [register_count]
    int32_t*       reg_shapes;      // Packed shapes [register_count * SF_MAX_DIMS]
    uint8_t*       ownership_flags; // [register_count] 1 if data is owned by state, 0 if view
    size_t         register_count;
    sf_allocator*  allocator;
    
    // Backend-specific prepared execution plan
    void* baked_data;

    // Error flag set by execution contexts.
    // 0 = No Error. Uses sf_exec_error codes.
    sf_atomic_i32  error_code;
    sf_atomic_i32* global_error_ptr; // Points to engine->error_code for global Kill Switch
} sf_state;

#endif // SF_STATE_H