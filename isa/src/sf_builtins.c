#include <sionflow/isa/sf_builtins.h>
#include <string.h>
#include <stdlib.h>

void sf_provider_parse(const char* provider, u16* out_builtin_id, u8* out_builtin_axis) {
    if (!provider || provider[0] == '\0') {
        if (out_builtin_id) *out_builtin_id = (u16)SF_BUILTIN_NONE;
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

    if (out_builtin_id) *out_builtin_id = (u16)SF_BUILTIN_NONE;
    if (out_builtin_axis) *out_builtin_axis = 0;
}
