#ifndef SF_BUILTINS_H
#define SF_BUILTINS_H

#include <sionflow/base/sf_types.h>
#include <sionflow/isa/sf_builtins.inc>

typedef enum {
#define SF_BUILTIN(id, name) SF_BUILTIN_##id,
    SF_BUILTIN_LIST
#undef SF_BUILTIN
    SF_BUILTIN_COUNT
} sf_builtin_id;

/**
 * @brief Parses a provider string (e.g. "host.index.0") into a builtin ID and axis.
 */
void sf_provider_parse(const char* provider, u16* out_builtin_id, u8* out_builtin_axis);

#endif // SF_BUILTINS_H
