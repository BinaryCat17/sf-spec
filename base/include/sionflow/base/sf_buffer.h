#ifndef SF_BUFFER_H
#define SF_BUFFER_H

#include "sf_types.h"
#include "sf_memory.h"

// Flags for buffer properties
#define SF_BUFFER_OWNS_DATA (1 << 0) // Buffer is responsible for freeing 'data'
#define SF_BUFFER_GPU       (1 << 1) // Data resides in VRAM (future)
#define SF_BUFFER_PINNED    (1 << 2) // CPU memory pinned for DMA (future)

typedef struct {
    void* data;            // Pointer to raw memory
    size_t size_bytes;     // Total allocated size
    
    sf_allocator* alloc;   // Allocator used for this buffer (ref, not owned)
    u32 flags;
    u32 ref_count;         // For shared ownership (future proofing)
} sf_buffer;

// Initialize a buffer from existing memory (does not own data)
void sf_buffer_init_view(sf_buffer* buf, void* data, size_t size);

// Allocate a new buffer (owns data)
// Returns false on allocation failure
bool sf_buffer_alloc(sf_buffer* buf, sf_allocator* alloc, size_t size);

// Free buffer memory if it owns it. Does not free the 'sf_buffer' struct itself.
void sf_buffer_free(sf_buffer* buf);

#endif // SF_BUFFER_H
