#include <sionflow/base/sf_shape.h>
#include <sionflow/base/sf_log.h>
#include <stdio.h>
#include <string.h>

void sf_shape_calc_strides(sf_type_info* info) {
    int32_t stride = 1;
    for (int k = (int)info->ndim - 1; k >= 0; --k) {
        info->strides[k] = stride;
        stride *= (info->shape[k] > 0 ? info->shape[k] : 1);
    }
}

size_t sf_shape_calc_count(const int32_t* shape, uint8_t ndim) {
    if (ndim == 0) return 1;
    size_t count = 1;
    for (int i = 0; i < ndim; ++i) {
        count *= (shape[i] > 0 ? (size_t)shape[i] : 1);
    }
    return count;
}

size_t sf_shape_calc_bytes(sf_dtype dtype, const int32_t* shape, uint8_t ndim) {
    return sf_shape_calc_count(shape, ndim) * sf_dtype_size(dtype);
}

bool sf_shape_is_scalar(const sf_type_info* info) {
    if (info->ndim == 0) return true;
    for (int i = 0; i < info->ndim; ++i) {
        if (info->shape[i] > 1) return false;
    }
    return true;
}

void sf_shape_normalize(sf_type_info* info) {
    if (info->ndim == 0) return;
    
    int32_t new_shape[SF_MAX_DIMS];
    uint8_t new_ndim = 0;
    
    for (int i = 0; i < info->ndim; ++i) {
        if (info->shape[i] != 1) {
            new_shape[new_ndim++] = info->shape[i];
        }
    }
    
    info->ndim = new_ndim;
    if (new_ndim > 0) {
        memcpy(info->shape, new_shape, sizeof(int32_t) * new_ndim);
    }
    sf_shape_calc_strides(info);
}

void sf_shape_get_broadcast_strides(const sf_type_info* tensor, const sf_type_info* domain, int32_t* out_strides) {
    for (int i = 0; i < SF_MAX_DIMS; ++i) out_strides[i] = 0;
    
    // Scalar tensor has 0 strides in all domain dimensions
    if (sf_shape_is_scalar(tensor)) return;

    // Temporary tensor to get standard contiguous strides
    sf_type_info t = *tensor;
    sf_shape_calc_strides(&t);

    int t_idx = (int)tensor->ndim - 1;
    int d_idx = (int)domain->ndim - 1;

    while (d_idx >= 0) {
        if (t_idx >= 0) {
            // If dimensions match, take the tensor's native stride
            if (tensor->shape[t_idx] == domain->shape[d_idx]) {
                out_strides[d_idx] = t.strides[t_idx];
            } else if (tensor->shape[t_idx] == 1) {
                // If tensor dimension is 1, it broadcasts (stride 0)
                out_strides[d_idx] = 0;
            } else {
                // Incompatible dimensions should have been caught by compiler, 
                // but we safely set to 0 here.
                out_strides[d_idx] = 0;
            }
            t_idx--;
        } else {
            // Tensor rank is smaller than domain rank: it broadcasts (stride 0)
            out_strides[d_idx] = 0;
        }
        d_idx--;
    }
}

void sf_shape_format(const sf_type_info* info, char* buf, size_t size) {
    if (info->ndim == 0) {
        snprintf(buf, size, "[]");
        return;
    }
    int offset = snprintf(buf, size, "[");
    for (int i = 0; i < info->ndim; ++i) {
        if (offset >= (int)size) break;
        offset += snprintf(buf + offset, size - offset, "%d%s", info->shape[i], i < (int)info->ndim - 1 ? "," : "");
    }
    if (offset < (int)size) snprintf(buf + offset, size - offset, "]");
}

bool sf_shape_broadcast(const sf_type_info* a, const sf_type_info* b, sf_type_info* out) {
    if (sf_shape_is_scalar(a)) { *out = *b; return true; }
    if (sf_shape_is_scalar(b)) { *out = *a; return true; }
    
    int ndim_a = (int)a->ndim;
    int ndim_b = (int)b->ndim;
    int max_ndim = (ndim_a > ndim_b) ? ndim_a : ndim_b;
    
    out->ndim = (uint8_t)max_ndim;
    out->dtype = a->dtype;
    
    for (int i = 0; i < max_ndim; ++i) {
        int idx_a = ndim_a - 1 - i;
        int idx_b = ndim_b - 1 - i;
        int idx_out = max_ndim - 1 - i;
        
        int32_t dim_a = (idx_a >= 0) ? a->shape[idx_a] : 1;
        int32_t dim_b = (idx_b >= 0) ? b->shape[idx_b] : 1;
        
        if (dim_a == dim_b) out->shape[idx_out] = dim_a;
        else if (dim_a == 1) out->shape[idx_out] = dim_b;
        else if (dim_b == 1) out->shape[idx_out] = dim_a;
        else if (dim_a < 0 || dim_b < 0) out->shape[idx_out] = (dim_a > 0) ? dim_a : dim_b;
        else return false;
    }
    
    sf_shape_calc_strides(out);
    return true;
}
// dev mode test
