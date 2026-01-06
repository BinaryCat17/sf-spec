#include <sionflow/base/sf_json.h>
#include <sionflow/base/sf_utils.h>
#include <sionflow/base/sf_log.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// --- Lexer ---

typedef enum {
    TOK_EOF,
    TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_COLON, TOK_COMMA,
    TOK_STRING, TOK_NUMBER,
    TOK_TRUE, TOK_FALSE, TOK_NULL
} sf_token_type;

typedef struct {
    sf_token_type type;
    const char* start;
    size_t length;
    sf_json_loc loc;
} sf_token;

typedef struct {
    const char* source;
    const char* cursor;
    sf_json_loc loc;
} sf_lexer;

static void lex_init(sf_lexer* l, const char* source) {
    l->source = source;
    l->cursor = source;
    l->loc.line = 1;
    l->loc.column = 1;
}

static void skip_whitespace(sf_lexer* l) {
    while (*l->cursor) {
        char c = *l->cursor;
        if (isspace(c)) {
            if (c == '\n') {
                l->loc.line++;
                l->loc.column = 1;
            } else {
                l->loc.column++;
            }
            l->cursor++;
        } else if (c == '/' && l->cursor[1] == '/') {
            while (*l->cursor && *l->cursor != '\n') {
                l->cursor++;
                l->loc.column++;
            }
        } else {
            break;
        }
    }
}

static sf_token next_token(sf_lexer* l) {
    skip_whitespace(l);
    
    sf_token t;
    t.start = l->cursor;
    t.loc = l->loc;
    t.length = 1;

    char c = *l->cursor;
    if (!c) {
        t.type = TOK_EOF;
        return t;
    }

    l->cursor++;
    l->loc.column++;

    switch (c) {
        case '{': t.type = TOK_LBRACE; return t;
        case '}': t.type = TOK_RBRACE; return t;
        case '[': t.type = TOK_LBRACKET; return t;
        case ']': t.type = TOK_RBRACKET; return t;
        case ':': t.type = TOK_COLON; return t;
        case ',': t.type = TOK_COMMA; return t;
        case '"':
            t.type = TOK_STRING;
            t.start = l->cursor;
            while (*l->cursor && *l->cursor != '"') {
                if (*l->cursor == '\\' && l->cursor[1]) {
                    l->cursor += 2;
                    l->loc.column += 2;
                } else {
                    l->cursor++;
                    l->loc.column++;
                }
            }
            t.length = l->cursor - t.start;
            if (*l->cursor == '"') {
                l->cursor++;
                l->loc.column++;
            }
            return t;
    }

    if (isdigit(c) || c == '-') {
        t.type = TOK_NUMBER;
        t.start = l->cursor - 1;
        while (*l->cursor && (isdigit(*l->cursor) || *l->cursor == '.' || *l->cursor == 'e' || *l->cursor == 'E' || *l->cursor == '+' || *l->cursor == '-')) {
            l->cursor++;
            l->loc.column++;
        }
        t.length = l->cursor - t.start;
        return t;
    }

    if (isalpha(c)) {
        t.start = l->cursor - 1;
        while (*l->cursor && isalpha(*l->cursor)) {
            l->cursor++;
            l->loc.column++;
        }
        t.length = l->cursor - t.start;
        if (strncmp(t.start, "true", 4) == 0 && t.length == 4) t.type = TOK_TRUE;
        else if (strncmp(t.start, "false", 5) == 0 && t.length == 5) t.type = TOK_FALSE;
        else if (strncmp(t.start, "null", 4) == 0 && t.length == 4) t.type = TOK_NULL;
        else t.type = TOK_EOF; // Error
        return t;
    }

    t.type = TOK_EOF;
    return t;
}

// --- Parser ---

typedef struct {
    sf_lexer lexer;
    sf_token peek;
    sf_arena* arena;
} sf_parser;

static void advance(sf_parser* p) {
    p->peek = next_token(&p->lexer);
}

static sf_json_value* parse_value(sf_parser* p);

static sf_json_value* parse_object(sf_parser* p) {
    sf_json_value* v = SF_ARENA_PUSH(p->arena, sf_json_value, 1);
    v->type = SF_JSON_VAL_OBJECT;
    v->loc = p->peek.loc;
    
    advance(p); // {
    
    typedef struct Field {
        const char* key;
        sf_json_value* val;
        struct Field* next;
    } Field;
    
    Field* head = NULL;
    Field* tail = NULL;
    size_t count = 0;

    while (p->peek.type != TOK_RBRACE && p->peek.type != TOK_EOF) {
        if (p->peek.type != TOK_STRING) {
            SF_LOG_ERROR("Expected string key at %u:%u", p->peek.loc.line, p->peek.loc.column);
            return v;
        }
        
        char* key = sf_arena_alloc((sf_allocator*)p->arena, p->peek.length + 1);
        memcpy(key, p->peek.start, p->peek.length);
        key[p->peek.length] = '\0';
        advance(p);
        
        if (p->peek.type != TOK_COLON) {
            SF_LOG_ERROR("Expected ':' after key at %u:%u", p->peek.loc.line, p->peek.loc.column);
            return v;
        }
        advance(p);
        
        sf_json_value* val = parse_value(p);
        
        Field* f = SF_ARENA_PUSH(p->arena, Field, 1);
        f->key = key;
        f->val = val;
        f->next = NULL;
        
        if (tail) tail->next = f; else head = f;
        tail = f;
        count++;
        
        if (p->peek.type == TOK_COMMA) advance(p);
        else if (p->peek.type != TOK_RBRACE) {
            SF_LOG_ERROR("Expected ',' or '}' at %u:%u", p->peek.loc.line, p->peek.loc.column);
            break;
        }
    }
    
    if (p->peek.type == TOK_RBRACE) advance(p);
    
    v->as.object.count = count;
    v->as.object.keys = SF_ARENA_PUSH(p->arena, const char*, count);
    v->as.object.values = SF_ARENA_PUSH(p->arena, sf_json_value, count);
    
    size_t i = 0;
    for (Field* f = head; f; f = f->next) {
        v->as.object.keys[i] = f->key;
        v->as.object.values[i] = *f->val;
        i++;
    }
    
    return v;
}

static sf_json_value* parse_array(sf_parser* p) {
    sf_json_value* v = SF_ARENA_PUSH(p->arena, sf_json_value, 1);
    v->type = SF_JSON_VAL_ARRAY;
    v->loc = p->peek.loc;
    
    advance(p); // [
    
    typedef struct Item {
        sf_json_value* val;
        struct Item* next;
    } Item;
    
    Item* head = NULL;
    Item* tail = NULL;
    size_t count = 0;
    
    while (p->peek.type != TOK_RBRACKET && p->peek.type != TOK_EOF) {
        sf_json_value* val = parse_value(p);
        
        Item* it = SF_ARENA_PUSH(p->arena, Item, 1);
        it->val = val;
        it->next = NULL;
        
        if (tail) tail->next = it; else head = it;
        tail = it;
        count++;
        
        if (p->peek.type == TOK_COMMA) advance(p);
        else if (p->peek.type != TOK_RBRACKET) {
            SF_LOG_ERROR("Expected ',' or ']' at %u:%u", p->peek.loc.line, p->peek.loc.column);
            break;
        }
    }
    
    if (p->peek.type == TOK_RBRACKET) advance(p);
    
    v->as.array.count = count;
    v->as.array.items = SF_ARENA_PUSH(p->arena, sf_json_value, count);
    
    size_t i = 0;
    for (Item* it = head; it; it = it->next) {
        v->as.array.items[i++] = *it->val;
    }
    
    return v;
}

static sf_json_value* parse_value(sf_parser* p) {
    sf_json_value* v = NULL;
    switch (p->peek.type) {
        case TOK_LBRACE: return parse_object(p);
        case TOK_LBRACKET: return parse_array(p);
        case TOK_STRING:
            v = SF_ARENA_PUSH(p->arena, sf_json_value, 1);
            v->type = SF_JSON_VAL_STRING;
            v->loc = p->peek.loc;
            char* s = sf_arena_alloc((sf_allocator*)p->arena, p->peek.length + 1);
            memcpy(s, p->peek.start, p->peek.length);
            s[p->peek.length] = '\0';
            v->as.s = s;
            advance(p);
            return v;
        case TOK_NUMBER:
            v = SF_ARENA_PUSH(p->arena, sf_json_value, 1);
            v->type = SF_JSON_VAL_NUMBER;
            v->loc = p->peek.loc;
            char buf[64];
            size_t len = p->peek.length < 63 ? p->peek.length : 63;
            memcpy(buf, p->peek.start, len);
            buf[len] = '\0';
            v->as.n = atof(buf);
            advance(p);
            return v;
        case TOK_TRUE:
            v = SF_ARENA_PUSH(p->arena, sf_json_value, 1);
            v->type = SF_JSON_VAL_BOOL;
            v->loc = p->peek.loc;
            v->as.b = true;
            advance(p);
            return v;
        case TOK_FALSE:
            v = SF_ARENA_PUSH(p->arena, sf_json_value, 1);
            v->type = SF_JSON_VAL_BOOL;
            v->loc = p->peek.loc;
            v->as.b = false;
            advance(p);
            return v;
        case TOK_NULL:
            v = SF_ARENA_PUSH(p->arena, sf_json_value, 1);
            v->type = SF_JSON_VAL_NULL;
            v->loc = p->peek.loc;
            advance(p);
            return v;
        default:
            SF_LOG_ERROR("Unexpected token at %u:%u", p->peek.loc.line, p->peek.loc.column);
            advance(p);
            return NULL;
    }
}

// --- API Implementation ---

const sf_json_value* sf_json_get_field(const sf_json_value* obj, const char* key) {
    if (!obj || obj->type != SF_JSON_VAL_OBJECT) return NULL;
    for (size_t i = 0; i < obj->as.object.count; ++i) {
        if (strcmp(obj->as.object.keys[i], key) == 0) {
            return &obj->as.object.values[i];
        }
    }
    return NULL;
}

const char* sf_json_get_string(const sf_json_value* val) {
    if (val && val->type == SF_JSON_VAL_STRING) return val->as.s;
    return NULL;
}

sf_json_value* sf_json_parse(const char* json_str, sf_arena* arena) {
    sf_parser p;
    lex_init(&p.lexer, json_str);
    p.arena = arena;
    advance(&p);
    return parse_value(&p);
}

sf_ast_graph* sf_json_parse_graph(const char* json_str, sf_arena* arena) {
    sf_json_value* root_val = sf_json_parse(json_str, arena);
    if (!root_val || root_val->type != SF_JSON_VAL_OBJECT) return NULL;
    
    sf_ast_graph* graph = SF_ARENA_PUSH(arena, sf_ast_graph, 1);
    memset(graph, 0, sizeof(sf_ast_graph));
    graph->root = root_val;
    
    const sf_json_value* nodes_val = sf_json_get_field(root_val, "nodes");
    if (nodes_val && nodes_val->type == SF_JSON_VAL_ARRAY) {
        graph->node_count = nodes_val->as.array.count;
        graph->nodes = SF_ARENA_PUSH(arena, sf_ast_node, graph->node_count);
        for (size_t i = 0; i < graph->node_count; ++i) {
            const sf_json_value* n_val = &nodes_val->as.array.items[i];
            sf_ast_node* node = &graph->nodes[i];
            node->loc = n_val->loc;
            
            const sf_json_value* id_val = sf_json_get_field(n_val, "id");
            node->id = (id_val && id_val->type == SF_JSON_VAL_STRING) ? id_val->as.s : "unknown";
            
            const sf_json_value* type_val = sf_json_get_field(n_val, "type");
            node->type = (type_val && type_val->type == SF_JSON_VAL_STRING) ? type_val->as.s : "unknown";
            
            const sf_json_value* data_val = sf_json_get_field(n_val, "data");
            node->data = (sf_json_value*)data_val;
        }
    }
    
    const sf_json_value* links_val = sf_json_get_field(root_val, "links");
    if (links_val && links_val->type == SF_JSON_VAL_ARRAY) {
        graph->link_count = links_val->as.array.count;
        graph->links = SF_ARENA_PUSH(arena, sf_ast_link, graph->link_count);
        for (size_t i = 0; i < graph->link_count; ++i) {
            const sf_json_value* l_val = &links_val->as.array.items[i];
            sf_ast_link* link = &graph->links[i];
            link->loc = l_val->loc;
            
            link->src = sf_json_get_string(sf_json_get_field(l_val, "src"));
            if (!link->src) link->src = "unknown";
            
            link->dst = sf_json_get_string(sf_json_get_field(l_val, "dst"));
            if (!link->dst) link->dst = "unknown";
            
            link->src_port = sf_json_get_string(sf_json_get_field(l_val, "src_port"));
            link->dst_port = sf_json_get_string(sf_json_get_field(l_val, "dst_port"));
        }
    }

    const sf_json_value* imports_val = sf_json_get_field(root_val, "imports");
    if (imports_val && imports_val->type == SF_JSON_VAL_ARRAY) {
        graph->import_count = imports_val->as.array.count;
        graph->imports = SF_ARENA_PUSH(arena, const char*, graph->import_count);
        for (size_t i = 0; i < graph->import_count; ++i) {
            const sf_json_value* imp = &imports_val->as.array.items[i];
            graph->imports[i] = (imp->type == SF_JSON_VAL_STRING) ? imp->as.s : "";
        }
    }
    
    return graph;
}
            