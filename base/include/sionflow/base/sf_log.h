#ifndef SF_LOG_H
#define SF_LOG_H

#include <sionflow/base/sf_platform.h>
#include <stdarg.h>

// --- Log Levels ---

typedef enum {
    SF_LOG_LEVEL_FATAL = 0,
    SF_LOG_LEVEL_ERROR = 1,
    SF_LOG_LEVEL_WARN  = 2,
    SF_LOG_LEVEL_INFO  = 3,
    SF_LOG_LEVEL_DEBUG = 4,
    SF_LOG_LEVEL_TRACE = 5
} sf_log_level;

// --- Sink Interface ---

/*
 * Callback function for log output.
 * user_data: Custom pointer passed during registration (e.g. FILE* or WindowHandle).
 * level: The severity of the message.
 * file/line: Source location.
 * message: The formatted log message (null-terminated).
 */
typedef void (*sf_log_sink_fn)(void* user_data, sf_log_level level, const char* file, int line, const char* message);

// --- Public API ---

/**
 * Initializes the logging system. 
 * Not strictly required if zero-initialization is sufficient, but good for explicit setup.
 */
void sf_log_init(void);

/**
 * Clean up resources (mutexes).
 */
void sf_log_shutdown(void);

/**
 * Register a new log sink.
 * sink_fn: The callback to invoke.
 * user_data: Context pointer passed to the callback.
 * level: The maximum log level this sink cares about.
 */
void sf_log_add_sink(sf_log_sink_fn sink_fn, void* user_data, sf_log_level level);

/**
 * Helper: Adds a file sink.
 * filename: Path to the log file.
 * level: The maximum log level.
 */
void sf_log_add_file_sink(const char* filename, sf_log_level level);

/**
 * Helper: Sets the global "gatekeeper" level manually (rarely needed, usually auto-managed).
 */
void sf_log_set_global_level(sf_log_level level);

/**
 * Internal function called by macros. Do not call directly.
 */
void sf_log_message(sf_log_level level, const char* file, int line, const char* fmt, ...);

// --- Internal Global State for Zero-Cost checks ---

extern sf_log_level g_sf_log_global_level;

// --- Macros ---

// The "do { ... } while(0)" idiom ensures the macro behaves like a single statement.
// We check the global level *before* evaluating arguments to avoid formatting overhead.

#define SF_LOG_FATAL(fmt, ...) do { \
    if (g_sf_log_global_level >= SF_LOG_LEVEL_FATAL) \
        sf_log_message(SF_LOG_LEVEL_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
} while(0)

#define SF_LOG_ERROR(fmt, ...) do { \
    if (g_sf_log_global_level >= SF_LOG_LEVEL_ERROR) \
        sf_log_message(SF_LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
} while(0)

#define SF_LOG_WARN(fmt, ...) do { \
    if (g_sf_log_global_level >= SF_LOG_LEVEL_WARN) \
        sf_log_message(SF_LOG_LEVEL_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
} while(0)

#define SF_LOG_INFO(fmt, ...) do { \
    if (g_sf_log_global_level >= SF_LOG_LEVEL_INFO) \
        sf_log_message(SF_LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
} while(0)

#define SF_LOG_DEBUG(fmt, ...) do { \
    if (g_sf_log_global_level >= SF_LOG_LEVEL_DEBUG) \
        sf_log_message(SF_LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
} while(0)

#define SF_LOG_TRACE(fmt, ...) do { \
    if (g_sf_log_global_level >= SF_LOG_LEVEL_TRACE) \
        sf_log_message(SF_LOG_LEVEL_TRACE, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
} while(0)

#endif // SF_LOG_H
