// Copyright (c) 2008, Willow Garage, Inc.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Author: Josh Faust

#ifndef RCUTILS__ASSERT_H_
#define RCUTILS__ASSERT_H_

#include "rcutils/logging_macros.h"

/** \file */

/** \def RCUTILS_ASSERT(cond)
 * \brief Asserts that the provided condition evaluates to true.
 *
 * If it is false, program execution will abort, with an informative
 * statement about which assertion failed, in what file.  Use RCUTILS_ASSERT
 * instead of assert() itself.
 *
 * If running inside a debugger, RCUTILS_ASSERT will allow you to step past the assertion.
 */

/** \def RCUTILS_ASSERT_MSG(cond, ...)
 * \brief Asserts that the provided condition evaluates to true.
 *
 * If it is false, program execution will abort, with an informative
 * statement about which assertion failed, in what file, and it will print out
 * a printf-style message you define.  Example usage:
 @verbatim
   RCUTILS_ASSERT_MSG(x > 0, "Uh oh, x went negative.  Value = %d", x);
 @endverbatim
 *
 * If running inside a debugger, RCUTILS_ASSERT will allow you to step past the assertion.
 */

/**
 * \def RCUTILS_ASSERT_CMD()
 * \brief Runs a command if the provided condition is false
 *
 * For example:
\verbatim
  RCUTILS_ASSERT_CMD(x > 0, handleError(...));
\endverbatim
 */

/** \def RCUTILS_BREAK()
 * \brief Aborts program execution.
 *
 * Aborts program execution with an informative message stating what file and
 * line it was called from. Use RCUTILS_BREAK instead of calling assert(0) or
 * RCUTILS_ASSERT(0).
 *
 * If running inside a debugger, RCUTILS_BREAK will allow you to step past the breakpoint.
 */

/** \def RCUTILS_ISSUE_BREAK()
 * \brief Always issues a breakpoint instruction.
 *
 * This define is mostly for internal use, but is useful if you want to simply issue a break
 * instruction in a cross-platform way.
 *
 * Currently implemented for Windows (any platform), powerpc64, and x86
 */


#ifdef WIN32
# if defined (__MINGW32__)
#  define RCUTILS_ISSUE_BREAK() DebugBreak();
# else  // MSVC
#  define RCUTILS_ISSUE_BREAK() __debugbreak();
# endif
#elif defined(__powerpc64__)
# define RCUTILS_ISSUE_BREAK() asm volatile ("tw 31,1,1");
#elif defined(__i386__) || defined(__ia64__) || defined(__x86_64__)
# define RCUTILS_ISSUE_BREAK() asm ("int $3");
#else
# include <stdlib.h>
# define RCUTILS_ISSUE_BREAK() abort();
#endif

#ifndef NDEBUG
#ifndef RCUTILS_ASSERT_ENABLED
#define RCUTILS_ASSERT_ENABLED
#endif
#endif

#ifdef RCUTILS_ASSERT_ENABLED
#define RCUTILS_BREAK() \
  do { \
    RCUTILS_LOG_FATAL("BREAKPOINT HIT\n\tfile = %s\n\tline=%d\n", __FILE__, __LINE__); \
    RCUTILS_ISSUE_BREAK() \
  } while (false)

#define RCUTILS_ASSERT(cond) \
  do { \
    if (!(cond)) { \
      RCUTILS_LOG_FATAL("ASSERTION FAILED\n\tfile = %s\n\tline = %d\n\tcond = %s\n", \
        __FILE__, __LINE__, #cond); \
      RCUTILS_ISSUE_BREAK() \
    } \
  } while (false)

#define RCUTILS_ASSERT_MSG(cond, ...) \
  do { \
    if (!(cond)) { \
      RCUTILS_LOG_FATAL("ASSERTION FAILED\n\tfile = %s\n\tline = %d\n\tcond = %s\n\tmessage = ", \
        __FILE__, __LINE__, #cond); \
      RCUTILS_LOG_FATAL(__VA_ARGS__); \
      RCUTILS_LOG_FATAL("\n"); \
      RCUTILS_ISSUE_BREAK(); \
    } \
  } while (false)

#else
#define RCUTILS_BREAK()
#define RCUTILS_ASSERT(cond)
#define RCUTILS_ASSERT_MSG(cond, ...)
#endif

#endif  // RCUTILS__ASSERT_H_
