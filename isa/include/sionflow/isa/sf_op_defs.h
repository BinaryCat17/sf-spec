#ifndef SF_OP_DEFS_H
#define SF_OP_DEFS_H

#include <sionflow/base/sf_types.h>
#include <sionflow/isa/sf_isa_constants.h>

typedef enum {
    SF_ASSERT_NONE = 0,
    SF_ASSERT_MATCH_DIM,
    SF_ASSERT_BROADCAST_COMPATIBLE
} sf_assert_type;

typedef struct {
    sf_assert_type type;
    int8_t p0, a0; // Port index and Axis (-1 for last)
    int8_t p1, a1; // Port index and Axis (-1 for last)
    const char* msg;
} sf_op_assert;

/**
 * @brief Unified metadata for an operation.
 */
typedef struct {
    const char* name;
    uint16_t opcode;    // sf_opcode
    uint8_t category;   // sf_op_category
    uint8_t strategy;   // sf_dispatch_strategy
    
    uint32_t input_mask;
    uint32_t output_mask;
    
    uint8_t shape_rule; // sf_shape_rule
    uint8_t out_rule;   // sf_out_rule
    uint8_t access;     // sf_access_pattern
    
    const char* ports[4]; 
    uint8_t arity;
    int8_t min_rank;
    int8_t max_rank;
    uint16_t flags;
    
    const sf_op_assert* assertions;
    uint8_t assertion_count;
} sf_op_metadata;

#define SF_OP_FLAG_SPATIAL     (1 << 0)
#define SF_OP_FLAG_REDUCER     (1 << 1)
#define SF_OP_FLAG_GENERATOR   (1 << 2)
#define SF_OP_FLAG_MEMORY      (1 << 3)
#define SF_OP_FLAG_FORCE_DOM   (1 << 4)
#define SF_OP_FLAG_COMMUTATIVE  (1 << 5)
#define SF_OP_FLAG_ASSOCIATIVE  (1 << 6)

extern const sf_op_metadata SF_OP_METADATA[];

#endif // SF_OP_DEFS_H
    