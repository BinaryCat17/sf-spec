#include <sionflow/isa/sf_exec_ctx.h>
#include <string.h>

void sf_exec_ctx_init(sf_exec_ctx* ctx, sf_allocator* allocator) {
    memset(ctx, 0, sizeof(sf_exec_ctx));
    ctx->allocator = allocator;
    ctx->ndim = 1; 
    ctx->tile_size[0] = 1;
    ctx->domain_shape[0] = 1;
    ctx->batch_size = 1;
    ctx->global_error_ptr = NULL;
}

void* sf_exec_ctx_scratch_alloc(sf_exec_ctx* ctx, size_t size) {
    if (!ctx || !ctx->allocator) return NULL;
    return ctx->allocator->alloc(ctx->allocator, size);
}

sf_tensor* sf_exec_ctx_scratch_tensor(sf_exec_ctx* ctx, const sf_type_info* info) {
    if (!ctx || !ctx->allocator) return NULL;
    sf_tensor* t = (sf_tensor*)ctx->allocator->alloc(ctx->allocator, sizeof(sf_tensor));
    if (!t) return NULL;
    if (!sf_tensor_alloc(t, ctx->allocator, info)) return NULL;
    return t;
}
