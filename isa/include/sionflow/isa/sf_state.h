#ifndef SF_STATE_H
#define SF_STATE_H

#include <sionflow/isa/sf_tensor.h>
#include <sionflow/base/sf_memory.h>
#include <sionflow/base/sf_platform.h>

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

    // Task-specific pre-calculated linear strides
    int32_t* task_strides; // [register_count]

    // Error flag set by execution contexts.
    // 0 = No Error. Uses sf_exec_error codes.
    sf_atomic_i32  error_code;
    sf_atomic_i32* global_error_ptr; // Points to engine->error_code for global Kill Switch
} sf_state;

#endif // SF_STATE_H