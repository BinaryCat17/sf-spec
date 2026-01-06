#ifndef SF_BACKEND_H
#define SF_BACKEND_H

#include <sionflow/isa/sf_opcodes.h>
#include <sionflow/isa/sf_state.h>

// Forward declarations
struct sf_exec_ctx;
struct sf_program;
struct sf_task;

/**
 * @brief Backend Interface.
 * Handles the execution of a program over a N-dimensional domain.
 */

// Hook for sync (e.g. GPU upload/download)
// Called by the runtime when a tensor is mapped.
typedef void (*sf_hook_map)(void* impl, sf_tensor* tensor, sf_access_mode mode);

/**
 * @brief Dispatch function for a backend.
 */
typedef void (*sf_backend_dispatch_func)(
    void* backend_state,
    const struct sf_program* program,
    sf_state* state,
    const sf_tensor* domain,
    const struct sf_task* task
);

// Bake function to prepare a program for execution (pre-calculates plans, etc.)
typedef void* (*sf_backend_bake_func)(void* backend_state, const struct sf_program* program);

// Cleanup function for baked program data
typedef void (*sf_backend_free_baked_func)(void* backend_state, void* baked_data);

// Cleanup function for backend resources
typedef void (*sf_backend_shutdown_func)(void* backend_state);

typedef struct {
    // Internal Backend State (Opaque to Engine)
    void* state;

    sf_hook_map on_map;
    
    sf_backend_bake_func bake;
    sf_backend_free_baked_func free_baked;
    sf_backend_dispatch_func dispatch;
    sf_backend_shutdown_func shutdown;
} sf_backend;

#endif // SF_BACKEND_H
