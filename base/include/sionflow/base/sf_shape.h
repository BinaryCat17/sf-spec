#ifndef SF_SHAPE_H
#define SF_SHAPE_H

#include <sionflow/base/sf_types.h>

/**
 * @brief Calculates total number of elements in a shape.
 */
size_t sf_shape_calc_count(const int32_t* shape, uint8_t ndim);

/**
 * @brief Calculates total bytes needed for a tensor.
 */
size_t sf_shape_calc_bytes(sf_dtype dtype, const int32_t* shape, uint8_t ndim);

/**
 * Checks if a shape is effectively a scalar (rank 0 or all dimensions are 1).
 */
bool sf_shape_is_scalar(const sf_type_info* info);

/**
 * Normalizes a shape by removing leading/trailing dimensions of size 1.
 * If target_rank is > 0, tries to reach that rank.
 */
void sf_shape_normalize(sf_type_info* info);

/**
 * @brief Calculates linear strides for a contiguous tensor based on its shape.
 */
void sf_shape_calc_strides(sf_type_info* info);

/**
 * @brief Formats a shape as a string (e.g. "[100, 200]").
 */
void sf_shape_format(const sf_type_info* info, char* buf, size_t size);

/**
 * Checks if two shapes can be broadcasted and returns the result in 'out'.
 * Returns true if successful, false if shapes are incompatible.
 */
bool   sf_shape_broadcast(const sf_type_info* a, const sf_type_info* b, sf_type_info* out);

/**
 * @brief Calculates N-Dimensional strides for a tensor relative to an execution domain.
 * Supports NumPy-style broadcasting rules.
 */
void sf_shape_get_broadcast_strides(const sf_type_info* tensor, const sf_type_info* domain, int32_t* out_strides);

#endif // SF_SHAPE_H
