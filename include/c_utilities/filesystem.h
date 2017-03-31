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

#ifndef C_UTILITIES__FILESYSTEM_H_
#define C_UTILITIES__FILESYSTEM_H_

#if __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "c_utilities/visibility_control.h"

/// Return current working directory
/**
 * \param[in] buffer Allocated string to store current directory path to
 * \param[in] max_length maximum length to be stored in buffer
 * \return bool True if success False otherwise
 */
C_UTILITIES_PUBLIC
bool
utilities_get_cwd(char * buffer, size_t max_length);

/// Check if the provided path points to a directory.
/**
 * \param[in] abs_path Absolute path to check.
 * \return bool True if directory False otherwise
 */
C_UTILITIES_PUBLIC
bool
utilities_is_directory(const char * abs_path);

/// Check if the provided path points to a directory.
/**
 * \param[in] abs_path Absolute path to check.
 * \return bool True if file False otherwise
 */
C_UTILITIES_PUBLIC
bool
utilities_is_file(const char * abs_path);

/// Check if the provided path points to an existing file.
/**
 * \param[in] abs_path Absolute path to check.
 * \return bool True if the path exists False otherwise
 */
C_UTILITIES_PUBLIC
bool
utilities_exists(const char * abs_path);

/// Check if the provided path points to a file/folder readable by current user.
/**
 * \param[in] abs_path Absolute path to check.
 * \return bool True if the file is readable False otherwise
 */
C_UTILITIES_PUBLIC
bool
utilities_is_readable(const char * file_abs_path);

/// Check if the provided path points to a file/folder writable by current user.
/**
 * \param[in] abs_path Absolute path to check.
 * \return bool True if the file is writable False otherwise
 */
C_UTILITIES_PUBLIC
bool
utilities_is_writable(const char * file_abs_path);

/// Check if the provided path points to a file/folder both readable and writable by current user.
/**
 * \param[in] abs_path Absolute path to check.
 * \return bool True if the file is redable and writable False otherwise
 */
C_UTILITIES_PUBLIC
bool
utilities_is_readable_and_writable(const char * file_abs_path);

#if __cplusplus
}
#endif

#endif  // C_UTILITIES__FILESYSTEM_H_
