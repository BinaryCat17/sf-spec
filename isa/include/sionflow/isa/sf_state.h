#ifndef SF_STATE_H
#define SF_STATE_H

#include <sionflow/isa/sf_tensor.h>
#include <sionflow/base/sf_memory.h>
#include <sionflow/base/sf_platform.h>

/**
 * @brief Execution Grid for N-Dimensional dispatch.
 */
typedef struct {
    uint32_t dims[SF_MAX_DIMS];       // Number of tiles in each dimension
    uint32_t tile_shape[SF_MAX_DIMS];  // Size of each tile
    uint32_t total_tiles;
} sf_grid;

/**
 * @brief Persistent container for tensor data and memory management.
 * Owned by the Engine. Backends read from/write to this state.
 */
typedef struct sf_state {
    sf_tensor* registers;
    uint8_t* ownership_flags; // [register_count] 1 if owned, 0 if view
    size_t register_count;
    sf_allocator* allocator;
    
    // Backend-specific prepared execution plan
    void* baked_data;

    // Task-specific pre-calculated N-D strides
    // Pointing to a block of [register_count * SF_MAX_DIMS] int32_t
    int32_t* task_strides; 
    
    // Execution grid for the current task
    sf_grid grid;

    // Error flag set by execution contexts.
    // 0 = No Error. Uses sf_exec_error codes.
    sf_atomic_i32  error_code;
    sf_atomic_i32* global_error_ptr; // Points to engine->error_code for global Kill Switch
} sf_state;

#endif // SF_STATE_H