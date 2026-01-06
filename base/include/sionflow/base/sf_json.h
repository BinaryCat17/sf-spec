#ifndef SF_JSON_H
#define SF_JSON_H

#include <sionflow/base/sf_types.h>
#include <sionflow/base/sf_memory.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    u32 line;
    u32 column;
} sf_json_loc;

typedef enum {
    SF_JSON_VAL_NULL,
    SF_JSON_VAL_BOOL,
    SF_JSON_VAL_NUMBER,
    SF_JSON_VAL_STRING,
    SF_JSON_VAL_ARRAY,
    SF_JSON_VAL_OBJECT
} sf_json_val_type;

typedef struct sf_json_value {
    sf_json_val_type type;
    union {
        bool b;
        double n;
        const char* s;
        struct {
            struct sf_json_value* items;
            size_t count;
        } array;
        struct {
            const char** keys;
            struct sf_json_value* values;
            size_t count;
        } object;
    } as;
    sf_json_loc loc;
} sf_json_value;

// --- Graph Specific AST (Legacy/Helper) ---
typedef struct {
    const char* id;
    const char* type;
    sf_json_value* data;
    sf_json_loc loc;
} sf_ast_node;

typedef struct {
    const char* src;
    const char* dst;
    const char* src_port;
    const char* dst_port;
    sf_json_loc loc;
} sf_ast_link;

typedef struct {
    sf_ast_node* nodes;
    size_t node_count;
    sf_ast_link* links;
    size_t link_count;
    const char** imports;
    size_t import_count;
    const char* source_path;
    const sf_json_value* root;
} sf_ast_graph;

// --- API ---

// Helper to get a value from an object value by key
const sf_json_value* sf_json_get_field(const sf_json_value* obj, const char* key);

// General JSON Parser
sf_json_value* sf_json_parse(const char* json_str, sf_arena* arena);

// Graph Specific Parser (Wraps sf_json_parse and extracts nodes/links)
sf_ast_graph* sf_json_parse_graph(const char* json_str, sf_arena* arena);

#endif // SF_JSON_H