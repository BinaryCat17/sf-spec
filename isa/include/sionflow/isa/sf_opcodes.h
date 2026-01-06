#ifndef SF_OPCODES_H
#define SF_OPCODES_H

#include <sionflow/base/sf_types.h>
#include <sionflow/isa/sf_op_defs.h>

// SionFlow Instruction Set Architecture (Opcodes)

#define SF_OP_LIMIT 1024

typedef enum {
#define SF_OPCODE(suffix, value) SF_OP_##suffix = value,
    SF_OPCODE_LIST
#undef SF_OPCODE

    // Special Markers (Optional, for validation)
    SF_OP_CORE_BEGIN  = 0,
    SF_OP_CORE_END    = 255,
    SF_OP_ARRAY_BEGIN = 256,
    SF_OP_ARRAY_END   = 511,
    SF_OP_STATE_BEGIN = 512,
    SF_OP_STATE_END   = 767,

} sf_opcode;

/**
 * @brief Runtime metadata for an operation.
 */
typedef struct {
    const char* name;
    const char* ports[4]; // Names of input ports (src1, src2, src3, src4)
} sf_runtime_op_metadata;

/**
 * @brief Returns a human-readable name for a given opcode.
 */
const char* sf_opcode_to_str(u16 opcode);

/**
 * @brief Returns runtime metadata for a given opcode.
 */
const sf_runtime_op_metadata* sf_get_op_metadata(u16 opcode);

#endif // SF_OPCODES_H