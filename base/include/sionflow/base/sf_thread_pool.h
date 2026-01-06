#ifndef SF_THREAD_POOL_H
#define SF_THREAD_POOL_H

#include <sionflow/base/sf_types.h>
#include <sionflow/base/sf_platform.h>

typedef struct sf_thread_pool sf_thread_pool;

/**
 * @brief Callback for thread-local initialization.
 * Called once per worker thread when the pool starts.
 * @return Pointer to thread-local data, passed to job_func.
 */
typedef void* (*sf_thread_init_func)(int thread_idx, void* user_data);

/**
 * @brief Callback for thread-local cleanup.
 * Called once per worker thread before the thread exits.
 */
typedef void (*sf_thread_cleanup_func)(void* thread_local_data, void* user_data);

/**
 * @brief The actual job to execute in parallel.
 * @param job_idx Index of the job [0..total_jobs-1].
 * @param thread_local_data Data returned by sf_thread_init_func for this thread.
 * @param user_data Passed to sf_thread_pool_run.
 */
typedef void (*sf_thread_job_func)(u32 job_idx, void* thread_local_data, void* user_data);

typedef struct sf_thread_pool_desc {
    int num_threads;             ///< Number of workers. 0 for auto (CPU count).
    sf_thread_init_func init_fn;    ///< Optional.
    sf_thread_cleanup_func cleanup_fn; ///< Optional.
    void* user_data;             ///< Passed to init/cleanup.
} sf_thread_pool_desc;

/**
 * @brief Creates a persistent thread pool.
 */
sf_thread_pool* sf_thread_pool_create(const sf_thread_pool_desc* desc);

/**
 * @brief Signals all threads to stop and joins them.
 */
void sf_thread_pool_destroy(sf_thread_pool* pool);

/**
 * @brief Runs a batch of jobs in parallel and blocks until all are finished.
 * @param pool The pool instance.
 * @param job_count Total number of jobs.
 * @param job_fn The function to execute.
 * @param user_data Passed to job_fn.
 */
void sf_thread_pool_run(
    sf_thread_pool* pool,
    u32 job_count,
    sf_thread_job_func job_fn,
    void* user_data
);

/**
 * @brief Returns the number of workers in the pool.
 */
int sf_thread_pool_get_thread_count(sf_thread_pool* pool);

#endif // SF_THREAD_POOL_H
