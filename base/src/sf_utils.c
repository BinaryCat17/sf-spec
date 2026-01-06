#include <sionflow/base/sf_utils.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// --- Hashing ---

u32 sf_fnv1a_hash(const char* str) {
    u32 hash = 2166136261u;
    while (*str) {
        hash ^= (u8)*str++;
        hash *= 16777619u;
    }
    return hash;
}

// --- String / Path Utils ---

char* sf_arena_strdup(sf_arena* arena, const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* copy = SF_ARENA_PUSH(arena, char, len + 1);
    strcpy(copy, str);
    return copy;
}

char* sf_arena_sprintf(sf_arena* arena, const char* fmt, ...) {
    va_list args;
    
    // 1. Calculate length
    va_start(args, fmt);
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    if (len < 0) return NULL;
    
    // 2. Allocate
    char* buf = SF_ARENA_PUSH(arena, char, len + 1);
    
    // 3. Print
    va_start(args, fmt);
    vsnprintf(buf, len + 1, fmt, args);
    va_end(args);
    
    return buf;
}

char* sf_path_get_dir(const char* path, sf_arena* arena) {
    // Find last slash
    const char* last_slash = strrchr(path, '/');
#ifdef _WIN32
    const char* last_bslash = strrchr(path, '\\');
    if (last_bslash > last_slash) last_slash = last_bslash;
#endif

    if (!last_slash) return sf_arena_strdup(arena, ".");

    size_t len = last_slash - path;
    char* dir = SF_ARENA_PUSH(arena, char, len + 1);
    memcpy(dir, path, len);
    dir[len] = '\0';
    return dir;
}

const char* sf_path_get_ext(const char* path) {
    if (!path) return "";
    const char* dot = strrchr(path, '.');
    if (!dot || dot == path) return "";
    return dot + 1;
}

char* sf_path_join(const char* dir, const char* file, sf_arena* arena) {
    if (!dir || !file) return NULL;
    
    // If file is absolute, return file
    if (file[0] == '/' || file[0] == '\\' || (strlen(file) > 2 && file[1] == ':')) {
        return sf_arena_strdup(arena, file);
    }

    size_t len1 = strlen(dir);
    
    // Check if dir already has trailing slash
    bool slash = (len1 > 0 && (dir[len1-1] == '/' || dir[len1-1] == '\\'));
    
    if (slash) {
        return sf_arena_sprintf(arena, "%s%s", dir, file);
    } else {
        return sf_arena_sprintf(arena, "%s/%s", dir, file);
    }
}

bool sf_file_exists(const char* path) {
    if (!path) return false;
    FILE* f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

char* sf_file_read(const char* path, sf_arena* arena) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Safety check
    if (len < 0) { fclose(f); return NULL; }
    
    char* buf = SF_ARENA_PUSH(arena, char, len + 1);
    if (fread(buf, 1, len, f) != (size_t)len) {
        // If read failed, strictly we can't rewind allocation in linear arena easily 
        // without save/restore, but for now just return NULL.
        fclose(f);
        return NULL;
    }
    buf[len] = 0;
    fclose(f);
    return buf;
}

void* sf_file_read_bin(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (len < 0) { fclose(f); return NULL; }
    
    void* buf = malloc((size_t)len);
    if (!buf) { fclose(f); return NULL; }
    
    if (fread(buf, 1, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return NULL;
    }
    
    fclose(f);
    if (out_size) *out_size = (size_t)len;
    return buf;
}

size_t sf_utf8_to_utf32(const char* utf8, u32* out_buffer, size_t max_out) {
    size_t count = 0;
    const unsigned char* p = (const unsigned char*)utf8;
    while (*p) {
        u32 codepoint = 0;
        int len = 0;
        
        if ((*p & 0x80) == 0) {
            codepoint = *p;
            len = 1;
        } else if ((*p & 0xE0) == 0xC0) {
            codepoint = (*p & 0x1F) << 6;
            codepoint |= (p[1] & 0x3F);
            len = 2;
        } else if ((*p & 0xF0) == 0xE0) {
            codepoint = (*p & 0x0F) << 12;
            codepoint |= (p[1] & 0x3F) << 6;
            codepoint |= (p[2] & 0x3F);
            len = 3;
        } else if ((*p & 0xF8) == 0xF0) {
            codepoint = (*p & 0x07) << 18;
            codepoint |= (p[1] & 0x3F) << 12;
            codepoint |= (p[2] & 0x3F) << 6;
            codepoint |= (p[3] & 0x3F);
            len = 4;
        } else {
            p++; 
            continue;
        }
        
        if (out_buffer && count < max_out) {
            out_buffer[count] = codepoint;
        }
        count++;
        p += len;
    }
    return count;
}

// --- String Map ---

void sf_map_init(sf_str_map* map, size_t capacity, sf_arena* arena) {
    map->capacity = capacity;
    map->count = 0;
    map->entries = SF_ARENA_PUSH(arena, sf_map_entry, capacity);
    memset(map->entries, 0, sizeof(sf_map_entry) * capacity);
}

void sf_map_put(sf_str_map* map, const char* key, u32 value) {
    if (map->count >= map->capacity / 2) return; 

    u32 hash = sf_fnv1a_hash(key);
    size_t idx = hash % map->capacity;

    while (map->entries[idx].key != NULL) {
        if (strcmp(map->entries[idx].key, key) == 0) {
            map->entries[idx].value = value;
            return;
        }
        idx = (idx + 1) % map->capacity;
    }

    map->entries[idx].key = key;
    map->entries[idx].value = value;
    map->count++;
}

void sf_map_put_ptr(sf_str_map* map, const char* key, void* ptr) {
    if (map->count >= map->capacity / 2) return; 

    u32 hash = sf_fnv1a_hash(key);
    size_t idx = hash % map->capacity;

    while (map->entries[idx].key != NULL) {
        if (strcmp(map->entries[idx].key, key) == 0) {
            map->entries[idx].ptr_value = ptr;
            return;
        }
        idx = (idx + 1) % map->capacity;
    }

    map->entries[idx].key = key;
    map->entries[idx].ptr_value = ptr;
    map->count++;
}

bool sf_map_get(sf_str_map* map, const char* key, u32* out_val) {
    if (map->capacity == 0) return false;
    u32 hash = sf_fnv1a_hash(key);
    size_t idx = hash % map->capacity;

    while (map->entries[idx].key != NULL) {
        if (strcmp(map->entries[idx].key, key) == 0) {
            if (out_val) *out_val = map->entries[idx].value;
            return true;
        }
        idx = (idx + 1) % map->capacity;
    }
    return false;
}

bool sf_map_get_ptr(sf_str_map* map, const char* key, void** out_ptr) {
    if (map->capacity == 0) return false;
    u32 hash = sf_fnv1a_hash(key);
    size_t idx = hash % map->capacity;

    while (map->entries[idx].key != NULL) {
        if (strcmp(map->entries[idx].key, key) == 0) {
            if (out_ptr) *out_ptr = map->entries[idx].ptr_value;
            return true;
        }
        idx = (idx + 1) % map->capacity;
    }
    return false;
}

void sf_provider_parse(const char* provider, u16* out_builtin_id, u8* out_builtin_axis) {
    if (!provider || provider[0] == '\0') {
        if (out_builtin_id) *out_builtin_id = SF_BUILTIN_NONE;
        if (out_builtin_axis) *out_builtin_axis = 0;
        return;
    }

#define SF_BUILTIN(id, name) \
    { \
        size_t len = strlen(name); \
        if (strncmp(provider, name, len) == 0) { \
            if (out_builtin_id) *out_builtin_id = (u16)SF_BUILTIN_##id; \
            if (out_builtin_axis) { \
                if (provider[len] == '.' && provider[len + 1] >= '0' && provider[len + 1] <= '9') { \
                    *out_builtin_axis = (u8)atoi(provider + len + 1); \
                } else { \
                    *out_builtin_axis = 0; \
                } \
            } \
            return; \
        } \
    }
    SF_BUILTIN_LIST
#undef SF_BUILTIN

    if (out_builtin_id) *out_builtin_id = SF_BUILTIN_NONE;
    if (out_builtin_axis) *out_builtin_axis = 0;
}

sf_dtype sf_dtype_from_str(const char* s) {
    if (!s) return SF_DTYPE_F32;
    
    // Case-insensitive comparison
    if (strcasecmp(s, "f32") == 0) return SF_DTYPE_F32;
    if (strcasecmp(s, "i32") == 0) return SF_DTYPE_I32;
    if (strcasecmp(s, "u8") == 0 || strcasecmp(s, "bool") == 0) return SF_DTYPE_U8;
    
    return SF_DTYPE_F32;
}
