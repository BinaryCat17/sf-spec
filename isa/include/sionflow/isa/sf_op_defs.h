#ifndef SF_OP_DEFS_H
#define SF_OP_DEFS_H

#include <sionflow/base/sf_types.h>

/**
 * SionFlow Operation Definitions (Metadata Structures)
 */

typedef enum {
    SF_OP_CAT_SPECIAL,     // Compiler intrinsic (Const, Input, Call, Copy, Output)
    SF_OP_CAT_ATOMIC,      // Primitive Math/Logic (1:1 element mapping)
    SF_OP_CAT_REDUCTION,   // Data reduction (Sum, Size, CumSum)
    SF_OP_CAT_ACCEL,       // High-performance accelerators (MatMul, Inverse)
    SF_OP_CAT_MEMORY,      // Layout & Random Access (Gather, Slice, Reshape, Filter)
} sf_op_category;

#define SF_TYPE_MASK_F32 (1 << SF_DTYPE_F32)
#define SF_TYPE_MASK_I32 (1 << SF_DTYPE_I32)
#define SF_TYPE_MASK_U8  (1 << SF_DTYPE_U8)
#define SF_TYPE_MASK_NUMERIC (SF_TYPE_MASK_F32 | SF_TYPE_MASK_I32)
#define SF_TYPE_MASK_ALL     (SF_TYPE_MASK_NUMERIC | SF_TYPE_MASK_U8)
#define SF_TYPE_MASK_LOGIC   (SF_TYPE_MASK_U8)

typedef enum {
    SF_OUT_SAME_AS_INPUT,   // Output follows s1 dtype (default)
    SF_OUT_SAME_AS_INPUT_2, // Output follows s2 dtype
    SF_OUT_FORCE_F32,       // Always F32
    SF_OUT_FORCE_U8,        // Always U8
    SF_OUT_FORCE_I32,       // Always I32
} sf_out_rule;

typedef enum {
    SF_SHAPE_SPECIAL,       // Handled individually (Const, Input, Call)
    SF_SHAPE_SAME_AS_S1,    // Output shape = Input 1 shape
    SF_SHAPE_SAME_AS_S2,    // Output shape = Input 2 shape
    SF_SHAPE_BROADCAST,     // Broadcast S1 and S2 (and S3 if present)
    SF_SHAPE_MATMUL,        // Matrix Multiplication [M,K] x [K,N] -> [M,N]
    SF_SHAPE_TRANSPOSE,     // Swap dim 0 and 1
    SF_SHAPE_DOT,           // Dot product (reduces last dim)
    SF_SHAPE_JOIN,          // Join/Concat (adds dimension)
    SF_SHAPE_GATHER,        // Shape follows indices
    SF_SHAPE_RESHAPE,       // Shape follows constant value
    SF_SHAPE_SLICE,         // 1D slice
    SF_SHAPE_SCALAR,        // Output is a single value (ndim=0)
    SF_SHAPE_COUNT
} sf_shape_rule;

typedef enum {
    SF_ACCESS_LINEAR,       // 1:1 element-wise mapping
    SF_ACCESS_WINDOW,       // Neighborhood access (Stencil/Relative)
    SF_ACCESS_RANDOM,       // Indirect access (Gather/Scatter)
    SF_ACCESS_GLOBAL,       // Full buffer access (Reductions)
    SF_ACCESS_SPECIAL,      // Handled by compiler (Const, Input, Call)
} sf_access_pattern;

typedef enum {
    SF_STRATEGY_DEFAULT,         // Simple parallel execution
    SF_STRATEGY_REDUCTION,       // Partial result per thread -> Final merge
    SF_STRATEGY_TWO_PASS_SYNC,   // Two passes with a barrier (e.g. CumSum)
} sf_dispatch_strategy;

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
    