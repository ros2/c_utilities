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

#include "gtest/gtest.h"

#include "./allocator_testing_utils.h"
#include "rcutils/error_handling.h"
#include "rcutils/types/string_array.h"

#ifdef _WIN32
  #define strdup _strdup
#endif

TEST(test_string_array, boot_string_array) {
  auto allocator = rcutils_get_default_allocator();
  auto failing_allocator = get_failing_allocator();
  rcutils_ret_t ret = RCUTILS_RET_OK;

  // UNDEFINED BEHAVIOR
  // rcutils_string_array_t sa00;
  // rcutils_string_array_fini(&sa00);

  rcutils_string_array_t sa0 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_fini(&sa0);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_string_array_init(NULL, 2, &allocator));
  rcutils_reset_error();
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_string_array_init(&sa0, 2, NULL));
  rcutils_reset_error();
  EXPECT_EQ(RCUTILS_RET_BAD_ALLOC, rcutils_string_array_init(&sa0, 2, &failing_allocator));
  rcutils_reset_error();

  rcutils_string_array_t sa1 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa1, 3, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_string_array_fini(&sa1);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  rcutils_string_array_t sa2 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa2, 2, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  sa2.data[0] = strdup("Hello");
  sa2.data[1] = strdup("World");
  ret = rcutils_string_array_fini(&sa2);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  rcutils_string_array_t sa3 = rcutils_get_zero_initialized_string_array();
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_string_array_init(&sa3, 3, &allocator));
  sa3.allocator.allocate = NULL;
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_string_array_fini(NULL));
  rcutils_reset_error();
  ASSERT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_string_array_fini(&sa3));
  rcutils_reset_error();
  sa3.allocator = allocator;
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_string_array_fini(&sa3));

  rcutils_string_array_t sa4 = rcutils_get_zero_initialized_string_array();
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_string_array_init(&sa4, 0, &allocator));
  ASSERT_EQ(0u, sa4.size);
  ASSERT_EQ(RCUTILS_RET_OK, rcutils_string_array_fini(&sa4));
}

TEST(test_string_array, string_array_cmp) {
  auto allocator = rcutils_get_default_allocator();
  rcutils_ret_t ret = RCUTILS_RET_OK;
  int res = 0;

  // Initialize some string arrays
  rcutils_string_array_t sa0 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa0, 3, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  sa0.data[0] = strdup("foo");
  sa0.data[1] = strdup("bar");
  sa0.data[2] = strdup("baz");

  rcutils_string_array_t sa1 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa1, 3, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  sa1.data[0] = strdup("foo");
  sa1.data[1] = strdup("bar");
  sa1.data[2] = strdup("baz");

  rcutils_string_array_t sa2 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa2, 3, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  sa2.data[0] = strdup("foo");
  sa2.data[1] = strdup("baz");
  sa2.data[2] = strdup("bar");

  rcutils_string_array_t sa3 = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&sa3, 2, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  sa3.data[0] = strdup("foo");
  sa3.data[1] = strdup("bar");

  rcutils_string_array_t empty_string_array = rcutils_get_zero_initialized_string_array();
  rcutils_string_array_t incomplete_string_array = rcutils_get_zero_initialized_string_array();
  ret = rcutils_string_array_init(&incomplete_string_array, 3, &allocator);
  ASSERT_EQ(RCUTILS_RET_OK, ret);

  // Test failure cases
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_string_array_cmp(NULL, &sa0, &res));
  rcutils_reset_error();
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_string_array_cmp(&sa0, NULL, &res));
  rcutils_reset_error();
  EXPECT_EQ(RCUTILS_RET_INVALID_ARGUMENT, rcutils_string_array_cmp(&sa0, &sa1, NULL));
  rcutils_reset_error();
  EXPECT_EQ(RCUTILS_RET_ERROR, rcutils_string_array_cmp(&sa0, &incomplete_string_array, &res));
  rcutils_reset_error();

  // Test success cases
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa0, &sa1, &res));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa1, &sa0, &res));
  EXPECT_EQ(res, 0);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa0, &sa2, &res));
  EXPECT_LT(res, 0);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa2, &sa0, &res));
  EXPECT_GT(res, 0);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa0, &sa3, &res));
  EXPECT_GT(res, 0);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa3, &sa0, &res));
  EXPECT_LT(res, 0);
  // Test transitivity
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa3, &sa2, &res));
  EXPECT_LT(res, 0);
  // Test empty
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&sa0, &empty_string_array, &res));
  EXPECT_GT(res, 0);
  EXPECT_EQ(RCUTILS_RET_OK, rcutils_string_array_cmp(&empty_string_array, &sa0, &res));
  EXPECT_LT(res, 0);

  ret = rcutils_string_array_fini(&sa0);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_string_array_fini(&sa1);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_string_array_fini(&sa2);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_string_array_fini(&sa3);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
  ret = rcutils_string_array_fini(&incomplete_string_array);
  ASSERT_EQ(RCUTILS_RET_OK, ret);
}
