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

#if __cplusplus
extern "C"
{
#endif

#include <string.h>

#include "rcutils/allocator.h"
#include "rcutils/find.h"
#include "rcutils/get_env.h"
#include "rcutils/logging.h"
#include "rcutils/snprintf.h"

#define RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN 2048

bool g_rcutils_logging_initialized = false;
char g_rcutils_logging_output_format_string[RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN];
static const char * g_rcutils_logging_default_output_format =
  "[{severity}] [{name}]: {message} ({function_name}() at {file_name}:{line_number})";

rcutils_logging_output_handler_t g_rcutils_logging_output_handler = NULL;

int g_rcutils_logging_severity_threshold = 0;

void rcutils_logging_initialize()
{
  if (!g_rcutils_logging_initialized) {
    g_rcutils_logging_output_handler = &rcutils_logging_console_output_handler;
    g_rcutils_logging_severity_threshold = RCUTILS_LOG_SEVERITY_INFO;

    // Check for the environment variable for custom output formatting
    const char * output_format;
    const char * ret = rcutils_get_env("RCUTILS_CONSOLE_OUTPUT_FORMAT", &output_format);
    if (NULL == ret && strcmp(output_format, "") != 0) {
      size_t chars_to_copy = strlen(output_format);
      if (chars_to_copy > RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN - 1) {
        chars_to_copy = RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN - 1;
      }
      memcpy(g_rcutils_logging_output_format_string, output_format, chars_to_copy);
      g_rcutils_logging_output_format_string[chars_to_copy] = '\0';
    } else {
      if (NULL != ret) {
        fprintf(
          stderr,
          "Failed to get output format from env. variable: %s. Using default output format.\n",
          ret);
      }
      memcpy(g_rcutils_logging_output_format_string, g_rcutils_logging_default_output_format,
        strlen(g_rcutils_logging_default_output_format) + 1);
    }
    g_rcutils_logging_initialized = true;
  }
}

rcutils_logging_output_handler_t rcutils_logging_get_output_handler()
{
  return g_rcutils_logging_output_handler;
}

void rcutils_logging_set_output_handler(rcutils_logging_output_handler_t function)
{
  g_rcutils_logging_output_handler = function;
}

int rcutils_logging_get_severity_threshold()
{
  return g_rcutils_logging_severity_threshold;
}

void rcutils_logging_set_severity_threshold(int severity)
{
  RCUTILS_LOGGING_AUTOINIT
    g_rcutils_logging_severity_threshold = severity;
}

void rcutils_log(
  rcutils_log_location_t * location,
  int severity, const char * name, const char * format, ...)
{
  if (severity < g_rcutils_logging_severity_threshold) {
    return;
  }
  rcutils_logging_output_handler_t output_handler = g_rcutils_logging_output_handler;
  if (output_handler) {
    va_list args;
    va_start(args, format);
    (*output_handler)(location, severity, name, format, &args);
    va_end(args);
  }
}

/// Ensures the logging buffer is large enough.
/**
 * Macro for (re)allocating a dynamic buffer if the ouput buffer doesn't have
 * enough space for an additional n characters and null terminator.
 * Whether to allocate or re-allocate is determined by if the output_buffer
 * points to the original static_output_buffer or not.
 * If a new buffer is allocated it will be null-terminated.
 *
 * /param n Number of characters requiring space (not including null terminator)
 * /param output_buffer_size Size allocated for the output buffer
 * /param allocator rcutils_allocator_t to use for (re)allocation
 * /param output_buffer The output buffer to ensure has enough space
 * /param static_output_buffer The static buffer initially used as the output
 */
#define RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER( \
    n, \
    output_buffer_size, \
    allocator, \
    output_buffer, \
    static_output_buffer \
) \
  size_t old_output_buffer_len = strlen(output_buffer); \
  size_t required_output_buffer_size = old_output_buffer_len + n + 1; \
  if (required_output_buffer_size > output_buffer_size) { \
    do { \
      output_buffer_size *= 2; \
    } while (required_output_buffer_size > output_buffer_size); \
    if (output_buffer == static_output_buffer) { \
      void * dynamic_output_buffer = allocator.allocate(output_buffer_size, allocator.state); \
      if (!dynamic_output_buffer) { \
        fprintf(stderr, "failed to allocate buffer for logging output\n"); \
        goto cleanup; \
      } \
      memcpy(dynamic_output_buffer, output_buffer, old_output_buffer_len); \
      output_buffer = (char *)dynamic_output_buffer; \
    } else { \
      void * new_dynamic_output_buffer = allocator.reallocate( \
        output_buffer, output_buffer_size, allocator.state); \
      if (!new_dynamic_output_buffer) { \
        fprintf(stderr, "failed to reallocate buffer for logging output\n"); \
        goto cleanup; \
      } \
      output_buffer = (char *)new_dynamic_output_buffer; \
    } \
    output_buffer[old_output_buffer_len] = '\0'; \
  }

void rcutils_logging_console_output_handler(
  rcutils_log_location_t * location,
  int severity, const char * name, const char * format, va_list * args)
{
  FILE * stream = NULL;
  const char * severity_string = "";
  switch (severity) {
    case RCUTILS_LOG_SEVERITY_DEBUG:
      stream = stdout;
      severity_string = "DEBUG";
      break;
    case RCUTILS_LOG_SEVERITY_INFO:
      stream = stdout;
      severity_string = "INFO";
      break;
    case RCUTILS_LOG_SEVERITY_WARN:
      stream = stderr;
      severity_string = "WARN";
      break;
    case RCUTILS_LOG_SEVERITY_ERROR:
      stream = stderr;
      severity_string = "ERROR";
      break;
    case RCUTILS_LOG_SEVERITY_FATAL:
      stream = stderr;
      severity_string = "FATAL";
      break;
    default:
      fprintf(stderr, "unknown severity level: %d\n", severity);
      return;
  }

  // Declare variables that will be needed for cleanup ahead of time.
  char static_output_buffer[1024];
  char * output_buffer = NULL;

  // Start with a fixed size message buffer and if during message formatting we need longer, we'll
  // dynamically allocate space.
  char static_message_buffer[1024];
  char * message_buffer = static_message_buffer;

  int written;
  {
    // use copy of args to keep args for potential later user
    va_list args_clone;
    va_copy(args_clone, *args);
    written = vsnprintf(static_message_buffer, sizeof(static_message_buffer), format, args_clone);
    va_end(args_clone);
  }
  if (written < 0) {
    fprintf(stderr, "failed to format message: '%s'\n", format);
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if ((size_t)written >= sizeof(static_message_buffer)) {
    // write was incomplete, allocate necessary memory dynamically
    size_t message_buffer_size = written + 1;
    void * dynamic_message_buffer = allocator.allocate(message_buffer_size, allocator.state);
    if (!dynamic_message_buffer) {
      fprintf(stderr, "failed to allocate buffer for message\n");
      return;
    }
    written = vsnprintf(dynamic_message_buffer, message_buffer_size, format, *args);
    message_buffer = (char *)dynamic_message_buffer;
    if (written < 0 || (size_t)written >= message_buffer_size) {
      fprintf(
        stderr,
        "failed to format message (using dynamically allocated memory): '%s'\n",
        format);
      goto cleanup;
    }
  }

  // Process the format string looking for known tokens.
  const char token_start_delimiter = '{';
  const char token_end_delimiter = '}';
  // Start with a fixed size output buffer and if during token expansion we need longer, we'll
  // dynamically allocate space.
  output_buffer = static_output_buffer;
  output_buffer[0] = '\0';
  size_t output_buffer_size = sizeof(static_output_buffer);
  const char * str = g_rcutils_logging_output_format_string;
  size_t size = strlen(g_rcutils_logging_output_format_string);

  // Walk through the format string and expand tokens when they're encountered.
  size_t i = 0;
  while (i < size) {
    // Print everything up to the next token start delimiter.
    size_t chars_to_start_delim = rcutils_find(str + i, token_start_delimiter);
    size_t remaining_chars = size - i;
    if (chars_to_start_delim > 0) {
      if (chars_to_start_delim > remaining_chars) {
        // No start delimiters found; don't allow printing more of than what's left if.
        chars_to_start_delim = remaining_chars;
      }
      RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
        chars_to_start_delim, output_buffer_size, allocator, output_buffer, static_output_buffer)
      memcpy(output_buffer + old_output_buffer_len, str + i, chars_to_start_delim);
      output_buffer[old_output_buffer_len + chars_to_start_delim] = '\0';
      i += chars_to_start_delim;
      if (i >= size) {
        break;
      }
    }
    // We are at a token start delimiter: determine if there's a known token or not.
    // Potential tokens can't possibly be longer than the format string itself.
    char token[RCUTILS_LOGGING_MAX_OUTPUT_FORMAT_LEN];
    // Look for a token end delimiter.
    size_t chars_to_end_delim = rcutils_find(str + i, token_end_delimiter);
    remaining_chars = size - i;
    if (chars_to_end_delim > remaining_chars) {
      // No end delimiters found in the remainder of the format string;
      // there won't be any more tokens so shortcut the rest of the checking.
      RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
        remaining_chars, output_buffer_size, allocator, output_buffer, static_output_buffer)
      memcpy(output_buffer + old_output_buffer_len, str + i, remaining_chars + 1);
      break;
    }
    // Found what looks like a token; determine if it's recognized.
    size_t token_len = chars_to_end_delim - 1;  // Not including delimiters.
    memcpy(token, str + i + 1, token_len);  // Skip the start delimiter.
    token[token_len] = '\0';
    const char * token_expansion = NULL;
    // The resulting token_expansion string must always be null-terminated.
    if (strcmp("severity", token) == 0) {
      token_expansion = severity_string;
    } else if (strcmp("name", token) == 0) {
      token_expansion = name;
    } else if (strcmp("message", token) == 0) {
      token_expansion = message_buffer;
    } else if (strcmp("function_name", token) == 0) {
      token_expansion = location ? location->function_name : "\"\"";
    } else if (strcmp("file_name", token) == 0) {
      token_expansion = location ? location->file_name : "\"\"";
    } else if (strcmp("line_number", token) == 0) {
      if (location) {
        // Allow 9 digits for the expansion (otherwise, truncate). Even in the case of truncation
        // the result will still be null-terminated.
        char line_number_expansion[10];
        written = rcutils_snprintf(
          line_number_expansion, sizeof(line_number_expansion), "%zu", location->line_number);
        if (written < 0) {
          fprintf(
            stderr,
            "failed to format line number: '%zu'\n",
            location->line_number);
          goto cleanup;
        }
        token_expansion = line_number_expansion;
      } else {
        token_expansion = "0";
      }
    } else {
      // This wasn't a token; print the start delimiter and continue the search as usual
      // (the substring might contain more start delimiters).
      RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
        1, output_buffer_size, allocator, output_buffer, static_output_buffer)
      memcpy(output_buffer + old_output_buffer_len, str + i, 1);
      output_buffer[old_output_buffer_len + 1] = '\0';
      i++;
      continue;
    }
    size_t n = strlen(token_expansion);
    RCUTILS_LOGGING_ENSURE_LARGE_ENOUGH_BUFFER(
      n, output_buffer_size, allocator, output_buffer, static_output_buffer)
    memcpy(output_buffer + old_output_buffer_len, token_expansion, n + 1);
    // Skip ahead to avoid re-processing the token characters (including the 2 delimiters).
    i += token_len + 2;
  }
  fprintf(stream, "%s\n", output_buffer);

cleanup:
  if (message_buffer && message_buffer != static_message_buffer) {
    allocator.deallocate(message_buffer, allocator.state);
  }

  if (output_buffer && output_buffer != static_output_buffer) {
    allocator.deallocate(output_buffer, allocator.state);
  }
}

#if __cplusplus
}
#endif
