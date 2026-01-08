#ifndef SF_UTILS_H
#define SF_UTILS_H

#include <sionflow/base/sf_memory.h>
#include <stdbool.h>

// --- Hashing ---

u32 sf_fnv1a_hash(const char* str);

// --- String / Path Utils ---

// Duplicates string into arena
char* sf_arena_strdup(sf_arena* arena, const char* str);

// Sprintf into arena
char* sf_arena_sprintf(sf_arena* arena, const char* fmt, ...);

// Extract directory from path (e.g. "a/b/c.json" -> "a/b")
char* sf_path_get_dir(const char* path, sf_arena* arena);

// Check if path is absolute
bool sf_path_is_absolute(const char* path);

// Extract extension from path (e.g. "a.json" -> "json")
const char* sf_path_get_ext(const char* path);

// Join directory and file (handling separators)
char* sf_path_join(const char* dir, const char* file, sf_arena* arena);

// Check if file exists
bool sf_file_exists(const char* path);

// Read entire file into arena memory (null-terminated)
char* sf_file_read(const char* path, sf_arena* arena);

// Read entire file as binary (no arena, caller must free)
void* sf_file_read_bin(const char* path, size_t* out_size);

// Decodes a UTF-8 string into UTF-32 codepoints.
// Returns the number of codepoints produced.
// If out_buffer is NULL, only calculates the count.
size_t sf_utf8_to_utf32(const char* utf8, u32* out_buffer, size_t max_out);

// --- String Map (Key -> U32) ---

typedef struct {
    const char* key;
    u32 value;
    void* ptr_value;
} sf_map_entry;

typedef struct {
    sf_map_entry* entries;
    size_t capacity;
    size_t count;
} sf_str_map;

void sf_map_init(sf_str_map* map, size_t capacity, sf_arena* arena);
void sf_map_put(sf_str_map* map, const char* key, u32 value);
void sf_map_put_ptr(sf_str_map* map, const char* key, void* ptr);
bool sf_map_get(sf_str_map* map, const char* key, u32* out_val);
bool sf_map_get_ptr(sf_str_map* map, const char* key, void** out_ptr);

#endif // SF_UTILS_H
