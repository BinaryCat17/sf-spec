#ifndef SF_TENSOR_H
#define SF_TENSOR_H

#include <sionflow/base/sf_types.h>
#include <sionflow/base/sf_buffer.h>
#include <sionflow/base/sf_memory.h>

// --- Tensor Structure ---

// A Tensor is a VIEW into a buffer.
typedef struct sf_tensor {
    sf_type_info info;   // Metadata: Shape, Strides, Type
    
    sf_buffer* buffer;   // Storage: Pointer to data owner
    size_t byte_offset;  // Offset in bytes from buffer->data
} sf_tensor;

// --- Helper Functions (Inline) ---

// Get raw pointer to data start (with offset applied)
static inline void* sf_tensor_data(const sf_tensor* t) {
    if (!t || !t->buffer || !t->buffer->data) return NULL;
    return (uint8_t*)t->buffer->data + t->byte_offset;
}

static inline bool sf_tensor_is_valid(const sf_tensor* t) {
    return t && t->buffer && t->buffer->data;
}

static inline bool sf_tensor_is_scalar(const sf_tensor* t) {
    return t->info.ndim == 0;
}

static inline size_t sf_tensor_count(const sf_tensor* t) {
    if (!t || t->info.ndim == 0) return 1;
    size_t count = 1;
    for(int i=0; i < t->info.ndim; ++i) {
        int32_t dim = t->info.shape[i];
        count *= (dim > 0 ? (size_t)dim : 0);
    }
    return count;
}

static inline size_t sf_tensor_size_bytes(const sf_tensor* t) {
    return sf_tensor_count(t) * sf_dtype_size(t->info.dtype);
}

static inline bool sf_tensor_same_shape(const sf_tensor* a, const sf_tensor* b) {
    if (a->info.ndim != b->info.ndim) return false;
    for (int i = 0; i < a->info.ndim; ++i) {
        if (a->info.shape[i] != b->info.shape[i]) return false;
    }
    return true;
}

// Check if the tensor data is contiguous in memory
static inline bool sf_tensor_is_contiguous(const sf_tensor* t) {
    if (t->info.ndim == 0) return true;
    if (t->info.ndim == 1) return t->info.strides[0] == 1 || t->info.shape[0] <= 1;
    
    int32_t stride = 1;
    for (int i = t->info.ndim - 1; i >= 0; --i) {
        if (t->info.strides[i] != stride) return false;
        stride *= (t->info.shape[i] > 0 ? t->info.shape[i] : 1);
    }
    return true;
}

// Calculate linear element offset from indices [i, j, k, ...]
static inline size_t sf_tensor_get_offset(const sf_tensor* t, const int32_t* indices) {
    size_t offset = 0;
    for (int i = 0; i < t->info.ndim; ++i) {
        offset += indices[i] * t->info.strides[i];
    }
    return offset;
}

// --- Tensor Operations ---

// Init tensor view pointing to an existing buffer
void sf_tensor_init(sf_tensor* tensor, sf_buffer* buf, const sf_type_info* info, size_t offset);

// Allocates a NEW buffer and sets up the tensor view to point to it (Offset 0)
bool sf_tensor_alloc(sf_tensor* tensor, sf_allocator* alloc, const sf_type_info* info);

// Resizes the underlying buffer (reallocation) OR creates a new buffer
// NOTE: This modifies the 'buffer' field. If the buffer was shared, this might detach logic.
bool sf_tensor_resize(sf_tensor* tensor, sf_allocator* allocator, const sf_type_info* new_info);

// Deep copy: Src -> Dst (allocates Dst if needed)
bool sf_tensor_copy_data(sf_tensor* dst, const sf_tensor* src);

// Shallow copy: Dst becomes a view of Src
void sf_tensor_view(sf_tensor* dst, const sf_tensor* src);

// Zero-Copy View Operations (O(1))
// Create a view into a subset of elements (1D slice for now, modifies byte_offset and shape)
bool sf_tensor_slice(sf_tensor* dst, const sf_tensor* src, size_t start_element, size_t count);

// Create a view with a different shape (must have same total element count)
bool sf_tensor_reshape(sf_tensor* dst, const sf_tensor* src, const int32_t* new_shape, int ndim);

// Create a view with swapped dimensions (modifies strides)
bool sf_tensor_transpose(sf_tensor* dst, const sf_tensor* src);

// --- Debugging ---

/**
 * @brief Prints tensor metadata and contents to stdout.
 * @param name Optional label for the tensor.
 * @param t The tensor to print.
 */
void sf_tensor_print(const char* name, const sf_tensor* t);

#endif // SF_TENSOR_H