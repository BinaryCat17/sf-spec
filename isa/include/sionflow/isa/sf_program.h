#ifndef SF_PROGRAM_H
#define SF_PROGRAM_H

#include <sionflow/base/sf_types.h>
#include "sf_instruction.h"
#include "sf_tensor.h"

#define SF_BINARY_MAGIC   0x4D464C57 // "MFLW"
#define SF_BINARY_VERSION 20         // Phase 9: The Cartridge Model (Container Format)

#define SF_MAX_SYMBOL_NAME 64
#define SF_MAX_TITLE_NAME 128
#define SF_MAX_SECTIONS    16

// Section Types
typedef enum {
    SF_SECTION_PROGRAM  = 0x01, // Compiled SionFlow Bytecode
    SF_SECTION_PIPELINE = 0x02, // Execution schedule and resource bindings (JSON)
    SF_SECTION_IMAGE    = 0x03, // Embedded Texture (Raw or Compressed)
    SF_SECTION_FONT     = 0x04, // Embedded SDF Font Data
    SF_SECTION_RAW      = 0x05, // Arbitrary data blob
} sf_section_type;

// Symbol Flags (for Port Mapping)
#define SF_SYMBOL_FLAG_INPUT  (1 << 6) // Read-Only (Bind to Front Buffer)
#define SF_SYMBOL_FLAG_OUTPUT (1 << 7) // Write-Only (Bind to Back Buffer)

// Tensor Flags
#define SF_TENSOR_FLAG_CONSTANT   (1 << 0)
#define SF_TENSOR_FLAG_REDUCTION  (1 << 1)
#define SF_TENSOR_FLAG_GENERATOR  (1 << 2)
#define SF_TENSOR_FLAG_ALIAS      (1 << 3) // Bound to external resource (Input/Output)
#define SF_TENSOR_FLAG_SPATIAL    (1 << 4) // Needs domain-sized buffer

// Binding Flags
#define SF_BINDING_FLAG_REDUCTION (1 << 0)

// --- Cartridge Container (Level 0) ---

typedef struct {
    char name[SF_MAX_SYMBOL_NAME];
    uint32_t type;   // sf_section_type
    uint32_t offset; // Offset from start of file
    uint32_t size;   // Size in bytes
    uint32_t reserved[4];
} sf_section_header;

typedef struct {
    u32 magic;             // 0x4D464C57
    u32 version;           // SF_BINARY_VERSION
    
    // App Settings
    char app_title[SF_MAX_TITLE_NAME];
    u32 window_width;
    u32 window_height;
    u32 num_threads;       // 0 = Auto
    u8 vsync;              // 1 = Enabled
    u8 fullscreen;         // 1 = Enabled
    u8 resizable;          // 1 = Enabled
    u8 reserved_flags[1];

    u32 section_count;
    sf_section_header sections[SF_MAX_SECTIONS];

    u32 reserved[8];       
} sf_cartridge_header;

// --- Program Section (Level 1) ---

// Map Name -> Register Index
typedef struct {
    char name[SF_MAX_SYMBOL_NAME];
    char provider[SF_MAX_SYMBOL_NAME];
    uint32_t name_hash; // FNV-1a
    uint32_t register_idx;
    uint32_t related_name_hash; // Hash of the Input symbol that drives this Output's shape (0 if none)
    uint8_t flags;       // SF_SYMBOL_FLAG_* | SF_RESOURCE_FLAG_*
    uint16_t builtin_id; // sf_builtin_id
    uint8_t builtin_axis; // For indexed providers like host.index.N
    uint8_t reserved[1];
} sf_bin_symbol;

// Binding between a register and a task's domain
typedef struct {
    uint16_t reg_idx;
    uint16_t flags;      // SF_BINDING_FLAG_*
    int32_t byte_stride; // Pre-calculated: stride * sizeof(dtype)
} sf_bin_task_binding;

// A single execution unit within a program (e.g. for a specific Output shape)
typedef struct {
    uint32_t start_inst;
    uint32_t inst_count;
    uint32_t domain_reg; // Index of the register that defines the execution domain (usually an Output)
    uint8_t strategy;    // sf_dispatch_strategy
    uint8_t reserved[3];
    
    uint32_t binding_offset; // Offset into global binding table
    uint32_t binding_count;  // Number of registers used in this task
} sf_task;

// Metadata for a single tensor in the binary file
typedef struct {
    uint8_t dtype;       // sf_dtype
    uint8_t ndim;        // Rank
    uint8_t is_constant; // 1 if data follows, 0 if uninitialized buffer
    uint8_t flags;       // SF_TENSOR_FLAG_*
    uint8_t reserved[4]; // Padding
    
    int32_t shape[SF_MAX_DIMS];
    
    uint64_t data_size;  // Size in bytes of the initial data (0 if not constant)
} sf_bin_tensor_desc;

// Header for a PROGRAM section
typedef struct {
    u32 instruction_count; 
    u32 tensor_count;      // Total number of registers/tensors
    u32 symbol_count;      // Number of named I/O entries (Resource Templates)
    u32 task_count;        // Number of execution tasks
    u32 binding_count;     // Total number of register bindings
    
    u32 reduction_scratch_size; // Elements needed for reductions
    u32 sync_scratch_size;      // Elements needed for sync operations
    
    u32 reserved[8];       
} sf_bin_header;

// In-memory representation of a single program
typedef struct sf_program {
    sf_bin_header meta;
    
    sf_instruction* code;
    
    // Array of descriptors and initial constant data
    sf_type_info* tensor_infos;
    void** tensor_data;
    
    uint8_t* tensor_flags; // Array of tensor flags

    sf_bin_symbol* symbols;
    sf_task* tasks;
    sf_bin_task_binding* bindings;
} sf_program;

#endif // SF_PROGRAM_H
