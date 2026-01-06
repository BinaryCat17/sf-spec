#ifndef SF_EXEC_CTX_H
#define SF_EXEC_CTX_H

#include <sionflow/isa/sf_tensor.h>
#include <sionflow/isa/sf_program.h>
#include <sionflow/base/sf_memory.h>
#include <sionflow/base/sf_platform.h>

// Forward decl
typedef struct sf_exec_ctx sf_exec_ctx;

// --- Execution State ---
typedef enum {
    SF_ERROR_NONE = 0,
    SF_ERROR_OOM = 1,          
    SF_ERROR_SHAPE_MISMATCH = 2, 
    SF_ERROR_INVALID_OP = 3,
    SF_ERROR_RUNTIME = 4,
    SF_ERROR_OUT_OF_BOUNDS = 5
} sf_exec_error;

static inline const char* sf_exec_error_to_str(sf_exec_error err) {
    switch (err) {
        case SF_ERROR_NONE:           return "NONE";
        case SF_ERROR_OOM:            return "OUT_OF_MEMORY";
        case SF_ERROR_SHAPE_MISMATCH: return "SHAPE_MISMATCH";
        case SF_ERROR_INVALID_OP:     return "INVALID_OPCODE";
        case SF_ERROR_RUNTIME:        return "RUNTIME_GENERIC_ERROR";
        case SF_ERROR_OUT_OF_BOUNDS:  return "OUT_OF_BOUNDS";
        default:                      return "UNKNOWN_ERROR";
    }
}

/**
 * @brief Light-weight execution context (Ephemeral).
 * Created on the stack or per-thread. Points to data in sf_state or tiled buffers.
 */
struct sf_exec_ctx {
    // Flat Execution Registry (Zero-Overhead Access)
    void* reg_ptrs[SF_MAX_REGISTERS];           // Base pointers for registers
    int32_t reg_strides[SF_MAX_REGISTERS];      // Pre-calculated byte strides for current task
    sf_type_info reg_info[SF_MAX_REGISTERS];    // Metadata for registers
    
    // Optional allocator for temporary allocations during execution
    sf_allocator* allocator; 
    
    // Execution Configuration
    u32 batch_size; 
    
    // N-Dimensional Context
    u8 ndim;
    u32 linear_offset;             // Linear start index of this tile
    u32 error_idx;                 // Element index (relative to tile start) where error occurred
    u32 tile_offset[SF_MAX_DIMS];  // Start coords of this tile/batch
    u32 tile_size[SF_MAX_DIMS];    // Size of this tile/batch (active elements)
    u32 domain_shape[SF_MAX_DIMS]; // Total size of the execution domain

    // State
    sf_exec_error error;
    sf_atomic_i32* global_error_ptr;
    
    // Sync Support (for multi-pass ops like CumSum)
    int sync_pass;
    void* sync_data;
    u32 job_idx;

    // User Data
    void* user_data;
};

// --- Execution Context API (Internal) ---

void sf_exec_ctx_init(sf_exec_ctx* ctx, sf_allocator* allocator);
void* sf_exec_ctx_scratch_alloc(sf_exec_ctx* ctx, size_t size);
sf_tensor* sf_exec_ctx_scratch_tensor(sf_exec_ctx* ctx, const sf_type_info* info);

#endif // SF_EXEC_CTX_H
