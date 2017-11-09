// Copyright 2017 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RCUTILS__LOGGING_H_
#define RCUTILS__LOGGING_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "rcutils/allocator.h"
#include "rcutils/types/rcutils_ret.h"
#include "rcutils/visibility_control.h"

#if __cplusplus
extern "C"
{
#endif

/// The flag if the logging system has been initialized.
RCUTILS_PUBLIC
extern bool g_rcutils_logging_initialized;

/// Initialize the logging system using the specified allocator.
/**
 * Initialize the logging system only if it was not in an initialized state.
 *
 * If an invalid allocator is passed, the initialization will fail.
 * Otherwise, this function will still set the internal state to initialized
 * even if an error occurs, to avoid repeated failing initialization attempts
 * since this function is called automatically from logging macros.
 * To re-attempt initialization, call rcutils_logging_shutdown() before
 * re-calling this function.
 *
 * If multiple errors occur, the error code of the last error will be returned.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param allocator rcutils_allocator_t to be used.
 * \return `RCUTILS_RET_OK` if successful.
 * \retrun `RCUTILS_RET_INVALID_ARGUMENT` if the allocator is invalid, in which
 *   case initialization will fail.
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if an error occurs reading the output
 *   format from the `RCUTILS_CONSOLE_OUTPUT_FORMAT` environment variable, in
 *   which case the default format will be used.
 * \return `RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID` if the internal logger
 *   severity map cannot be initialized, in which case logger severity
 *   thresholds will not be configurable.
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_logging_initialize_with_allocator(rcutils_allocator_t allocator);

/// Initialize the logging system.
/**
 * Call rcutils_logging_initialize_with_allocator() using the default allocator.
 * This function is called automatically when using the logging macros.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \return `RCUTILS_RET_OK` if successful.
 * \return `RCUTILS_RET_INVALID_ARGUMENT` if an error occurs reading the output
 *   format from the `RCUTILS_CONSOLE_OUTPUT_FORMAT` environment variable, in
 *   which case the default format will be used.
 * \return `RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID` if the internal logger
 *   severity map cannot be initialized, in which case logger severity
 *   thresholds will not be configurable.
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_logging_initialize();

/// Shutdown the logging system.
/**
 * Free the resources allocated for the logging system.
 * This puts the system into a state equivalent to being uninitialized.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \return `RCUTILS_RET_OK` if successful.
 * \return `RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID` if the internal logger
 *   severity map cannot be finalized.
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_logging_shutdown();

/// The structure identifying the caller location in the source code.
typedef struct rcutils_log_location_t
{
  /// The name of the function containing the log call.
  const char * function_name;
  /// The name of the source file containing the log call.
  const char * file_name;
  /// The line number containing the log call.
  size_t line_number;
} rcutils_log_location_t;

/// The severity levels of log messages / logger thresholds.
enum RCUTILS_LOG_SEVERITY
{
  RCUTILS_LOG_SEVERITY_UNSET = 0,  ///< The unset log level
  RCUTILS_LOG_SEVERITY_DEBUG = 10,  ///< The debug log level
  RCUTILS_LOG_SEVERITY_INFO = 20,  ///< The info log level
  RCUTILS_LOG_SEVERITY_WARN = 30,  ///< The warn log level
  RCUTILS_LOG_SEVERITY_ERROR = 40,  ///< The error log level
  RCUTILS_LOG_SEVERITY_FATAL = 50,  ///< The fatal log level
};

/// The names of severity levels.
extern const char * g_rcutils_log_severity_names[RCUTILS_LOG_SEVERITY_FATAL + 1];

/// The function signature to log messages.
/**
 * \param location The pointer to the location struct
 * \param severity The severity level
 * \param name The name of the logger
 * \param format The format string
 * \param args The variable argument list
 */
typedef void (* rcutils_logging_output_handler_t)(
  rcutils_log_location_t *,  // location
  int,  // severity
  const char *,  // name
  const char *,  // format
  va_list *  // args
);

/// The function pointer of the current output handler.
RCUTILS_PUBLIC
extern rcutils_logging_output_handler_t g_rcutils_logging_output_handler;

/// Get the current output handler.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \return The function pointer of the current output handler.
 */
RCUTILS_PUBLIC
rcutils_logging_output_handler_t rcutils_logging_get_output_handler();

/// Set the current output handler.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param function The function pointer of the output handler to be used.
 */
RCUTILS_PUBLIC
void rcutils_logging_set_output_handler(rcutils_logging_output_handler_t function);

/// The default severity threshold for log calls.
/**
 * This severity threshold is used for (1) nameless log calls and (2) named log
 * calls where the effective severity threshold of the name is unspecified.
 *
 * \see rcutils_logging_get_logger_effective_severity_threshold()
 */
RCUTILS_PUBLIC
extern int g_rcutils_logging_default_severity_threshold;

/// Get the default severity threshold for loggers.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \return The severity threshold.
 */
RCUTILS_PUBLIC
int rcutils_logging_get_default_severity_threshold();

/// Set the default severity threshold for loggers.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param severity The severity threshold to be used.
 */
RCUTILS_PUBLIC
void rcutils_logging_set_default_severity_threshold(int severity);

/// Get the severity threshold for a logger.
/**
 * This considers the severity threshold of the specifed logger only.
 * To get the effective severity threshold of a logger given the severity
 * threshold of its ancestors, see
 * rcutils_logging_get_logger_effective_severity_threshold().
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param name The name of the logger, must be null terminated c string
 * \return The severity threshold of the logger if it has been set, or
 * \return `RCUTILS_LOG_SEVERITY_UNSET` if unset, or
 * \return `g_rcutils_logging_default_severity_threshold` for an empty name, or
 * \return -1 on invalid arguments, or
 * \return -1 if an error occurred
 */
RCUTILS_PUBLIC
int rcutils_logging_get_logger_severity_threshold(const char * name);

/// Get the severity threshold for a logger and its name length.
/**
 * Identical to rcutils_logging_get_logger_severity_threshold() but without
 * relying on the logger name to be a null terminated c string.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param name The name of the logger
 * \param name_length Logger name length
 * \return The severity threshold of the logger if it has been set, or
 * \return `RCUTILS_LOG_SEVERITY_UNSET` if unset, or
 * \return `g_rcutils_logging_default_severity_threshold` for `name_length` of `0`, or
 * \return -1 on invalid arguments, or
 * \return -1 if an error occurred
 */
RCUTILS_PUBLIC
int rcutils_logging_get_logger_severity_thresholdn(const char * name, size_t name_length);

/// Set the severity threshold for a logger.
/**
 * If an empty string is specified as the name, the
 * `g_rcutils_logging_default_severity_threshold` will be set.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | Yes
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param name The name of the logger, must be null terminated c string.
 * \param severity The severity threshold to be used.
 * \return `RCUTILS_RET_OK` if successful, or
 * \return `RCUTILS_RET_INVALID_ARGUMENT` on invalid arguments, or
 * \return `RCUTILS_RET_LOGGING_SEVERITY_MAP_INVALID` if severity map invalid, or
 * \return `RCUTILS_RET_ERROR` if an unspecified error occured
 */
RCUTILS_PUBLIC
rcutils_ret_t rcutils_logging_set_logger_severity_threshold(const char * name, int severity);

/// Determine if a logger is enabled for a severity.
/**
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param name The name of the logger, must be null terminated c string or NULL.
 * \param severity The severity level.
 *
 * \return true if the logger is enabled for the severity; false otherwise.
 */
RCUTILS_PUBLIC
bool rcutils_logging_logger_is_enabled_for(const char * name, int severity);

/// Determine the effective severity threshold for a logger.
/**
 * The effective severity threshold is determined as the severity threshold of
 * the logger if it is set, otherwise it is the first specified severity
 * threshold of the logger's ancestors, starting with its closest ancestor.
 * The ancestor hierarchy is signified by logger names being separated by dots:
 * a logger named `x` is an ancestor of `x.y`, and both `x` and `x.y` are
 * ancestors of `x.y.z`, etc.
 * If the severity threshold has not been set for the logger nor any of its
 * ancestors, the default severity threshold is used.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, provided logging system is already initialized
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param name The name of the logger, must be null terminated c string.
 *
 * \return The severity threshold, or
 * \return -1 on invalid arguments, or
 * \return -1 if an error occurred.
 */
RCUTILS_PUBLIC
int rcutils_logging_get_logger_effective_severity_threshold(const char * name);

/// Log a message.
/**
 * The attributes of this function are also being influenced by the currently
 * set output handler.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No
 * Thread-Safe        | No
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param location The pointer to the location struct or NULL
 * \param severity The severity level
 * \param name The name of the logger, must be null terminated c string or NULL
 * \param format The format string
 * \param ... The variable arguments
 */
RCUTILS_PUBLIC
void rcutils_log(
  rcutils_log_location_t * location,
  int severity,
  const char * name,
  const char * format,
  ...);

/// The default output handler outputs log messages to the standard streams.
/**
 * The messages with a severity `DEBUG` and `INFO` are written to `stdout`.
 * The messages with a severity `WARN`, `ERROR`, and `FATAL` are written to
 * `stderr`.
 * The console output format of the logged message can be configured through
 * the `RCUTILS_CONSOLE_OUTPUT_FORMAT` environment variable.
 * By default, the severity and name is prepended and the location
 * information is appended.
 *
 * <hr>
 * Attribute          | Adherence
 * ------------------ | -------------
 * Allocates Memory   | No, for formatted outputs <= 1023 characters
 *                    | Yes, for formatted outputs >= 1024 characters
 * Thread-Safe        | Yes, if the underlying *printf functions are
 * Uses Atomics       | No
 * Lock-Free          | Yes
 *
 * \param location The pointer to the location struct or NULL
 * \param severity The severity level
 * \param name The name of the logger, must be null terminated c string
 * \param format The format string for the message contents
 * \param args The variable argument list for the message format string
 */
RCUTILS_PUBLIC
void rcutils_logging_console_output_handler(
  rcutils_log_location_t * location,
  int severity, const char * name, const char * format, va_list * args);

// Provide the compiler with branch prediction information
#ifndef _WIN32
/**
 * \def RCUTILS_LIKELY
 * Instruct the compiler to optimize for the case where the argument equals 1.
 */
# define RCUTILS_LIKELY(x) __builtin_expect((x), 1)
/**
 * \def RCUTILS_UNLIKELY
 * Instruct the compiler to optimize for the case where the argument equals 0.
 */
# define RCUTILS_UNLIKELY(x) __builtin_expect((x), 0)
#else
/**
 * \def RCUTILS_LIKELY
 * No op since Windows doesn't support providing branch prediction information.
 */
# define RCUTILS_LIKELY(x) (x)
/**
 * \def RCUTILS_UNLIKELY
 * No op since Windows doesn't support providing branch prediction information.
 */
# define RCUTILS_UNLIKELY(x) (x)
#endif  // _WIN32

/**
 * \def RCUTILS_LOGGING_AUTOINIT
 * \brief Initialize the rcl logging library.
 * Usually it is unnecessary to call the macro directly.
 * All logging macros ensure that this has been called once.
 */
#define RCUTILS_LOGGING_AUTOINIT \
  if (RCUTILS_UNLIKELY(!g_rcutils_logging_initialized)) { \
    rcutils_logging_initialize(); \
  }

#if __cplusplus
}
#endif

#endif  // RCUTILS__LOGGING_H_
