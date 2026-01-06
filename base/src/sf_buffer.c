#include <sionflow/base/sf_buffer.h>
#include <sionflow/base/sf_log.h>
#include <string.h>

void sf_buffer_init_view(sf_buffer* buf, void* data, size_t size) {
    if (!buf) return;
    buf->data = data;
    buf->size_bytes = size;
    buf->alloc = NULL;
    buf->flags = 0;
    buf->ref_count = 1;
}

bool sf_buffer_alloc(sf_buffer* buf, sf_allocator* alloc, size_t size) {
    if (!buf || !alloc) return false;
    
    void* mem = alloc->alloc(alloc, size);
    if (!mem) {
        SF_LOG_ERROR("Buffer allocation failed for size %zu", size);
        return false;
    }
    
    // Zero init for safety
    memset(mem, 0, size);
    
    buf->data = mem;
    buf->size_bytes = size;
    buf->alloc = alloc;
    buf->flags = SF_BUFFER_OWNS_DATA;
    buf->ref_count = 1;
    
    return true;
}

void sf_buffer_free(sf_buffer* buf) {
    if (!buf) return;
    
    if ((buf->flags & SF_BUFFER_OWNS_DATA) && buf->alloc && buf->data) {
        buf->alloc->free(buf->alloc, buf->data);
    }
    
    buf->data = NULL;
    buf->size_bytes = 0;
    buf->alloc = NULL;
    buf->flags = 0;
    buf->ref_count = 0;
}
