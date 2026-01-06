#ifndef SF_PLATFORM_H
#define SF_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    
    typedef HANDLE sf_thread_t;
    typedef CRITICAL_SECTION sf_mutex_t;
    typedef CONDITION_VARIABLE sf_cond_t;
    
    // Simple atomic for counter
    typedef volatile LONG sf_atomic_i32;

#else
    #include <pthread.h>
    #include <unistd.h>
    #include <stdatomic.h>
    
    typedef pthread_t sf_thread_t;
    typedef pthread_mutex_t sf_mutex_t;
    typedef pthread_cond_t sf_cond_t;
    
    typedef atomic_int sf_atomic_i32;
#endif

// Thread Function Prototype
typedef void* (*sf_thread_func)(void* arg);

// --- Thread API ---
int sf_thread_create(sf_thread_t* thread, sf_thread_func func, void* arg);
int sf_thread_join(sf_thread_t thread);
int sf_cpu_count(void);

// --- Mutex API ---
void sf_mutex_init(sf_mutex_t* mutex);
void sf_mutex_lock(sf_mutex_t* mutex);
void sf_mutex_unlock(sf_mutex_t* mutex);
void sf_mutex_destroy(sf_mutex_t* mutex);

// --- CondVar API ---
void sf_cond_init(sf_cond_t* cond);
void sf_cond_wait(sf_cond_t* cond, sf_mutex_t* mutex);
void sf_cond_signal(sf_cond_t* cond);
void sf_cond_broadcast(sf_cond_t* cond);
void sf_cond_destroy(sf_cond_t* cond);

// --- Atomic API ---
int32_t sf_atomic_inc(sf_atomic_i32* var);
int32_t sf_atomic_load(sf_atomic_i32* var);
void sf_atomic_store(sf_atomic_i32* var, int32_t val);

// --- File System API ---

/**
 * Creates a directory if it doesn't exist.
 * Returns true on success (or if exists), false on failure.
 */
bool sf_fs_mkdir(const char* path);

/**
 * Removes all files within a directory (non-recursive).
 * Returns true on success.
 */
bool sf_fs_clear_dir(const char* path);

#endif // SF_PLATFORM_H
