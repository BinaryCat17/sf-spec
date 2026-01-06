#include <sionflow/base/sf_platform.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>

// --- Windows Implementation ---

// Wrapper to match pthread signature
typedef struct {
    sf_thread_func func;
    void* arg;
} sf_win_thread_ctx;

static DWORD WINAPI win_thread_start(LPVOID lpParam) {
    sf_win_thread_ctx* ctx = (sf_win_thread_ctx*)lpParam;
    ctx->func(ctx->arg);
    free(ctx);
    return 0;
}

int sf_thread_create(sf_thread_t* thread, sf_thread_func func, void* arg) {
    sf_win_thread_ctx* ctx = malloc(sizeof(sf_win_thread_ctx));
    ctx->func = func;
    ctx->arg = arg;

    *thread = CreateThread(NULL, 0, win_thread_start, ctx, 0, NULL);
    return (*thread != NULL) ? 0 : 1;
}

int sf_thread_join(sf_thread_t thread) {
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    return 0;
}

int sf_cpu_count(void) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
}

void sf_mutex_init(sf_mutex_t* mutex) {
    InitializeCriticalSection(mutex);
}

void sf_mutex_lock(sf_mutex_t* mutex) {
    EnterCriticalSection(mutex);
}

void sf_mutex_unlock(sf_mutex_t* mutex) {
    LeaveCriticalSection(mutex);
}

void sf_mutex_destroy(sf_mutex_t* mutex) {
    DeleteCriticalSection(mutex);
}

void sf_cond_init(sf_cond_t* cond) {
    InitializeConditionVariable(cond);
}

void sf_cond_wait(sf_cond_t* cond, sf_mutex_t* mutex) {
    SleepConditionVariableCS(cond, mutex, INFINITE);
}

void sf_cond_signal(sf_cond_t* cond) {
    WakeConditionVariable(cond);
}

void sf_cond_broadcast(sf_cond_t* cond) {
    WakeAllConditionVariable(cond);
}

void sf_cond_destroy(sf_cond_t* cond) {
    // Windows Condition Variables do not need to be destroyed explicitly
    (void)cond;
}

int32_t sf_atomic_inc(sf_atomic_i32* var) {
    return InterlockedIncrement(var);
}

int32_t sf_atomic_load(sf_atomic_i32* var) {
    return *var; // Simple load on x86/x64 is atomic aligned
}

void sf_atomic_store(sf_atomic_i32* var, int32_t val) {
    InterlockedExchange(var, val);
}

// --- FS Windows ---

bool sf_fs_mkdir(const char* path) {
    int result = _mkdir(path);
    return (result == 0 || errno == EEXIST);
}

bool sf_fs_clear_dir(const char* path) {
    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s\*", path);

    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(search_path, &fd);

    if (hFind == INVALID_HANDLE_VALUE) return false;

    do {
        if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
            char file_path[MAX_PATH];
            snprintf(file_path, MAX_PATH, "%s\%s", path, fd.cFileName);
            
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                DeleteFile(file_path);
            }
        }
    } while (FindNextFile(hFind, &fd));

    FindClose(hFind);
    return true;
}

#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

// --- Linux/POSIX Implementation ---

int sf_thread_create(sf_thread_t* thread, sf_thread_func func, void* arg) {
    return pthread_create(thread, NULL, func, arg);
}

int sf_thread_join(sf_thread_t thread) {
    return pthread_join(thread, NULL);
}

int sf_cpu_count(void) {
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    return (nprocs < 1) ? 1 : (int)nprocs;
}

void sf_mutex_init(sf_mutex_t* mutex) {
    pthread_mutex_init(mutex, NULL);
}

void sf_mutex_lock(sf_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
}

void sf_mutex_unlock(sf_mutex_t* mutex) {
    pthread_mutex_unlock(mutex);
}

void sf_mutex_destroy(sf_mutex_t* mutex) {
    pthread_mutex_destroy(mutex);
}

void sf_cond_init(sf_cond_t* cond) {
    pthread_cond_init(cond, NULL);
}

void sf_cond_wait(sf_cond_t* cond, sf_mutex_t* mutex) {
    pthread_cond_wait(cond, mutex);
}

void sf_cond_signal(sf_cond_t* cond) {
    pthread_cond_signal(cond);
}

void sf_cond_broadcast(sf_cond_t* cond) {
    pthread_cond_broadcast(cond);
}

void sf_cond_destroy(sf_cond_t* cond) {
    pthread_cond_destroy(cond);
}

int32_t sf_atomic_inc(sf_atomic_i32* var) {
    return atomic_fetch_add(var, 1) + 1;
}

int32_t sf_atomic_load(sf_atomic_i32* var) {
    return atomic_load(var);
}

void sf_atomic_store(sf_atomic_i32* var, int32_t val) {
    atomic_store(var, val);
}

// --- FS POSIX ---

bool sf_fs_mkdir(const char* path) {
    int result = mkdir(path, 0755);
    return (result == 0 || errno == EEXIST);
}

bool sf_fs_clear_dir(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return false;

    struct dirent* entry;
    char full_path[1024];

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        remove(full_path);
    }
    closedir(dir);
    return true;
}

#endif