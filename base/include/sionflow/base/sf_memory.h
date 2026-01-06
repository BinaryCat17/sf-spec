#ifndef SF_MEMORY_H
#define SF_MEMORY_H

#include <sionflow/base/sf_types.h>
#include <stdlib.h>

// --- Allocator Interface ---

typedef struct sf_allocator sf_allocator;

struct sf_allocator {
    // Function pointers for polymorphism
    void* (*alloc)(sf_allocator* self, size_t size);
    void* (*realloc)(sf_allocator* self, void* ptr, size_t old_size, size_t new_size);
    void  (*free)(sf_allocator* self, void* ptr);
};

// --- Arena Allocator (Linear / Frame Memory) ---
// Fast, no free(), reset() only.

typedef struct sf_arena {
    sf_allocator base; // Inheritance
    u8* memory;
    size_t size;
    size_t pos;
} sf_arena;

void sf_arena_init(sf_arena* arena, void* backing_buffer, size_t size);
void* sf_arena_alloc(sf_allocator* self, size_t size); // Implements interface
void  sf_arena_reset(sf_arena* arena);

#define SF_ARENA_PUSH(arena, type, count) (type*)sf_arena_alloc((sf_allocator*)arena, sizeof(type) * (count))

// --- Heap Allocator (General Purpose) ---
// Supports alloc/free/realloc. Uses a Free List strategy.

typedef struct sf_heap_block sf_heap_block;

typedef struct {
    sf_allocator base;
    u8* memory;
    size_t size;
    sf_heap_block* free_list; // Head of the free blocks list
    
    // Stats
    size_t used_memory;       
    size_t peak_memory;
    size_t allocation_count;
} sf_heap;

void sf_heap_init(sf_heap* heap, void* backing_buffer, size_t size);
void* sf_heap_alloc(sf_allocator* self, size_t size);
void* sf_heap_realloc(sf_allocator* self, void* ptr, size_t old_size, size_t new_size);
void  sf_heap_free(sf_allocator* self, void* ptr);

#endif // SF_MEMORY_H
