// Copyright 2020 Open Source Robotics Foundation, Inc.
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

#include <gtest/gtest.h>

#include <sys/types.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#include <string>

#include "rcutils/strerror.h"

TEST(test_strerror, get_error) {
  // cleaning possible errors
  errno = 0;

  char error_string[1024];
  rcutils_strerror(error_string, sizeof(error_string));
#ifdef _WIN32
  ASSERT_STREQ(error_string, "No error");
#elif __APPLE__
  ASSERT_STREQ(error_string, "Undefined error: 0");
#else
  ASSERT_STREQ(error_string, "Success");
#endif
  // Set the error "No such file or directory"
  errno = 2;

  rcutils_strerror(error_string, sizeof(error_string));
  ASSERT_STREQ(error_string, "No such file or directory");
}
