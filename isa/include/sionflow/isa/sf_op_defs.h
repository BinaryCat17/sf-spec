#ifndef SF_OP_DEFS_H
#define SF_OP_DEFS_H

#include <sionflow/base/sf_types.h>
#include <sionflow/isa/sf_isa_constants.h>

/**
 * @brief Unified metadata for an operation.
 * Focused on Runtime Execution and structural graph properties.
 * (Validation and Shape Analysis logic is generated into compiler passes).
 */
typedef struct {
    const char* name;
    uint16_t opcode;    // sf_opcode
    uint8_t category;   // sf_op_category
    uint8_t strategy;   // sf_dispatch_strategy
    
    uint32_t input_mask;
    uint32_t out_rule;   // sf_out_rule
    uint8_t access;      // sf_access_pattern
    
    const char* ports[4]; 
    uint16_t flags;      // SF_OP_FLAG_*
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