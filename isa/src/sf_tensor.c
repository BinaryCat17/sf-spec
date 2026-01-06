#include <sionflow/isa/sf_tensor.h>
#include <sionflow/base/sf_log.h>
#include <string.h>

void sf_tensor_init(sf_tensor* tensor, sf_buffer* buf, const sf_type_info* info, size_t offset) {
    if (!tensor) return;
    if (info) tensor->info = *info;
    else memset(&tensor->info, 0, sizeof(sf_type_info));
    
    tensor->buffer = buf;
    tensor->byte_offset = offset;
}

bool sf_tensor_alloc(sf_tensor* tensor, sf_allocator* alloc, const sf_type_info* info) {
    if (!tensor || !alloc || !info) return false;
    
    tensor->info = *info;
    tensor->byte_offset = 0;
    
    // Allocate the sf_buffer structure itself
    sf_buffer* buf = (sf_buffer*)alloc->alloc(alloc, sizeof(sf_buffer));
    if (!buf) return false;
    
    size_t size_bytes = sf_tensor_size_bytes(tensor);
    if (!sf_buffer_alloc(buf, alloc, size_bytes)) {
        alloc->free(alloc, buf);
        return false;
    }
    
    tensor->buffer = buf;
    return true;
}

bool sf_tensor_resize(sf_tensor* tensor, sf_allocator* allocator, const sf_type_info* new_info) {
    if (!tensor || !allocator || !new_info) return false;
    
    size_t new_size_bytes = 1;
    for(int i=0; i<new_info->ndim; ++i) new_size_bytes *= (new_info->shape[i] > 0 ? new_info->shape[i] : 1);
    new_size_bytes *= sf_dtype_size(new_info->dtype);
    
    // Update metadata
    tensor->info = *new_info;
    
    // Check if we need to realloc
    if (!tensor->buffer) {
        return sf_tensor_alloc(tensor, allocator, new_info);
    }
    
    if (tensor->buffer->size_bytes < new_size_bytes) {
        void* new_data = allocator->alloc(allocator, new_size_bytes);
        if (!new_data) return false;
        
        memset(new_data, 0, new_size_bytes);
        
        if (tensor->buffer->data) {
            size_t copy_size = tensor->buffer->size_bytes;
            if (copy_size > new_size_bytes) copy_size = new_size_bytes;
            memcpy(new_data, tensor->buffer->data, copy_size);
            
            if ((tensor->buffer->flags & SF_BUFFER_OWNS_DATA) && tensor->buffer->alloc) {
                tensor->buffer->alloc->free(tensor->buffer->alloc, tensor->buffer->data);
            }
        }
        
        tensor->buffer->data = new_data;
        tensor->buffer->size_bytes = new_size_bytes;
        tensor->buffer->alloc = allocator;
        tensor->buffer->flags |= SF_BUFFER_OWNS_DATA;
    }
    
    return true;
}

bool sf_tensor_copy_data(sf_tensor* dst, const sf_tensor* src) {
    if (!dst || !src) return false;
    
    void* dst_ptr = sf_tensor_data(dst);
    void* src_ptr = sf_tensor_data(src);
    
    if (!dst_ptr || !src_ptr) return false;
    
    size_t count = sf_tensor_count(dst);
    size_t src_count = sf_tensor_count(src);
    if (count != src_count) return false; 

    size_t elem_size = sf_dtype_size(dst->info.dtype);

    if (sf_tensor_is_contiguous(dst) && sf_tensor_is_contiguous(src)) {
        memcpy(dst_ptr, src_ptr, count * elem_size);
        return true;
    }
    
    SF_LOG_ERROR("Tensor Copy: Non-contiguous tensors are no longer supported for direct copy. Use specialized kernels.");
    return false;
}

void sf_tensor_view(sf_tensor* dst, const sf_tensor* src) {
    if (!dst || !src) return;
    *dst = *src; // Copy struct (info + buffer ptr + offset)
}

bool sf_tensor_slice(sf_tensor* dst, const sf_tensor* src, size_t start_element, size_t count) {
    if (!dst || !src) return false;
    
    // Validations
    if (!sf_tensor_is_valid(src)) {
        SF_LOG_ERROR("Tensor Slice: Source tensor is invalid.");
        return false;
    }
    size_t src_count = sf_tensor_count(src);
    if (start_element + count > src_count) {
        SF_LOG_ERROR("Tensor Slice: Out of bounds. Start %zu + Count %zu > Source Count %zu", 
            start_element, count, src_count);
        return false;
    }

    // Create Base View
    sf_tensor_view(dst, src);
    
    // Modify
    size_t elem_size = sf_dtype_size(src->info.dtype);
    dst->byte_offset += start_element * elem_size;
    
    // Flatten shape to 1D for now (or keep dim 0?)
    // "View of Input[Start:End]" usually implies a flat slice or slicing dim 0.
    // If we assume flat slice:
    dst->info.ndim = 1;
    dst->info.shape[0] = (int32_t)count;
    dst->info.strides[0] = 1; // Contiguous
    
    return true;
}

bool sf_tensor_reshape(sf_tensor* dst, const sf_tensor* src, const int32_t* new_shape, int ndim) {
    if (!dst || !src || ndim > SF_MAX_DIMS) return false;
    
    // Count check
    size_t current_count = sf_tensor_count(src);
    size_t new_count = 1;
    for(int i=0; i<ndim; ++i) new_count *= new_shape[i];
    
    if (current_count != new_count) {
        SF_LOG_ERROR("Tensor Reshape: Count mismatch. Current %zu vs New %zu", current_count, new_count);
        return false;
    }
    
    // Create Base View
    sf_tensor_view(dst, src);
    
    // Modify Metadata
    sf_type_info_init_contiguous(&dst->info, src->info.dtype, new_shape, (uint8_t)ndim);
    
    return true;
}

bool sf_tensor_transpose(sf_tensor* dst, const sf_tensor* src) {
    if (!dst || !src) return false;
    
    // Only support 2D transpose for now
    if (src->info.ndim != 2) {
        SF_LOG_ERROR("Tensor Transpose: Only 2D tensors supported, got %dD", src->info.ndim);
        return false;
    }
    
    sf_tensor_view(dst, src);
    
    // Swap Shape and Strides
    dst->info.shape[0] = src->info.shape[1];
    dst->info.shape[1] = src->info.shape[0];
    dst->info.strides[0] = src->info.strides[1];
    dst->info.strides[1] = src->info.strides[0];
    
    return true;
}

#include <stdio.h>

void sf_tensor_print(const char* name, const sf_tensor* t) {
    if (!t) {
        printf("  %s: (NULL)\n", name ? name : "?");
        return;
    }
    void* data_ptr = (void*)sf_tensor_data(t);
    if (!data_ptr) {
        printf("  %s: (Empty)\n", name ? name : "?");
        return;
    }
    
    printf("  '%s' ", name ? name : "?"); 
    
    printf("Shape: [");
    for(int i=0; i<t->info.ndim; ++i) printf("%d%s", t->info.shape[i], i < t->info.ndim-1 ? "," : "");
    printf("] ");
    
    size_t count = sf_tensor_count(t);
    size_t limit = count > 16 ? 16 : count;
    
    if (!sf_tensor_is_contiguous(t)) {
        printf("(Non-contiguous, printing first %zu bytes as hex): ", limit * sf_dtype_size(t->info.dtype));
        u8* b = (u8*)data_ptr;
        for(size_t i=0; i<limit; ++i) printf("%02x ", b[i]);
        printf("\n");
        return;
    }

    if (t->info.dtype == SF_DTYPE_F32) {
        f32* p = (f32*)data_ptr;
        printf("F32: {");
        for(size_t i=0; i<limit; ++i) {
            printf("%.2f%s", p[i], i < limit-1 ? ", " : "");
        }
        if (count > limit) printf("... (+%zu)", count - limit);
        printf("}\n");
    } else if (t->info.dtype == SF_DTYPE_I32) {
        int32_t* p = (int32_t*)data_ptr;
        printf("I32: {");
        for(size_t i=0; i<limit; ++i) {
            printf("%d%s", p[i], i < limit-1 ? ", " : "");
        }
        if (count > limit) printf("... (+%zu)", count - limit);
        printf("}\n");
    } else if (t->info.dtype == SF_DTYPE_U8) {
        u8* p = (u8*)data_ptr;
        printf("Bool: {");
        for(size_t i=0; i<limit; ++i) {
            printf("%s%s", p[i] ? "true" : "false", i < limit-1 ? ", " : "");
        }
        printf("}\n");
    }
}
