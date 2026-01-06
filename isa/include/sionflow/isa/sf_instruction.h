#ifndef SF_INSTRUCTION_H
#define SF_INSTRUCTION_H

#include <sionflow/base/sf_types.h>
#include "sf_opcodes.h"

/**
 * @brief Standard 4-address code instruction.
 * 
 * Layout: [ Opcode (16) | Dest (16) | Src1 (16) | Src2 (16) | Src3 (16) | Src4 (16) ]
 * 
 * Strides are calculated by the compiler during the Analyze pass and stored 
 * directly in the instruction to minimize runtime overhead (STEP_N model).
 */
typedef struct sf_instruction {
    u16 opcode;
    u16 dest_idx;
    u16 src1_idx;
    u16 src2_idx;
    u16 src3_idx;
    u16 src4_idx;
    
    u16 line;
    u16 column;
} sf_instruction;

#endif // SF_INSTRUCTION_H
