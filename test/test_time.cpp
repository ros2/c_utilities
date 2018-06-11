// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include <chrono>
#include <cinttypes>
#include <thread>

#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"

#include "rcutils/error_handling.h"
#include "rcutils/time.h"

using osrf_testing_tools_cpp::memory_tools::disable_monitoring_in_all_threads;
using osrf_testing_tools_cpp::memory_tools::enable_monitoring_in_all_threads;
using osrf_testing_tools_cpp::memory_tools::on_unexpected_calloc;
using osrf_testing_tools_cpp::memory_tools::on_unexpected_free;
using osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc;
using osrf_testing_tools_cpp::memory_tools::on_unexpected_realloc;

class TestTimeFixture : public ::testing::Test
{
public:
  void SetUp()
  {
    osrf_testing_tools_cpp::memory_tools::initialize();
    on_unexpected_malloc([]() {FAIL() << "UNEXPECTED MALLOC";});
    on_unexpected_realloc([]() {FAIL() << "UNEXPECTED REALLOC";});
    on_unexpected_calloc([]() {FAIL() << "UNEXPECTED CALLOC";});
    on_unexpected_free([]() {FAIL() << "UNEXPECTED FREE";});
    enable_monitoring_in_all_threads();
  }

  void TearDown()
  {
    disable_monitoring_in_all_threads();
    osrf_testing_tools_cpp::memory_tools::uninitialize();
  }
};

// Tests the rcutils_system_time_now() function.
TEST_F(TestTimeFixture, test_rcutils_system_time_now) {
  rcutils_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcutils_system_time_now(nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  // Check for normal operation (not allowed to alloc).
  rcutils_time_point_value_t now = 0;
  EXPECT_NO_MEMORY_OPERATIONS({
    ret = rcutils_system_time_now(&now);
  });
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_NE(0u, now);
  // Compare to std::chrono::system_clock time (within a second).
  now = 0;
  ret = rcutils_system_time_now(&now);
  {
    std::chrono::system_clock::time_point now_sc = std::chrono::system_clock::now();
    auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now_sc.time_since_epoch());
    int64_t now_ns_int = now_ns.count();
    int64_t now_diff = now - now_ns_int;
    const int k_tolerance_ms = 1000;
    EXPECT_LE(llabs(now_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "system_clock differs";
  }
}

// Tests the rcutils_steady_time_now() function.
TEST_F(TestTimeFixture, test_rcutils_steady_time_now) {
  rcutils_ret_t ret;
  // Check for invalid argument error condition (allowed to alloc).
  ret = rcutils_steady_time_now(nullptr);
  EXPECT_EQ(ret, RCUTILS_RET_INVALID_ARGUMENT) << rcutils_get_error_string_safe();
  rcutils_reset_error();
  // Check for normal operation (not allowed to alloc).
  rcutils_time_point_value_t now = 0;
  EXPECT_NO_MEMORY_OPERATIONS({
    ret = rcutils_steady_time_now(&now);
  });
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  EXPECT_NE(0u, now);
  // Compare to std::chrono::steady_clock difference of two times (within a second).
  now = 0;
  EXPECT_NO_MEMORY_OPERATIONS({
    ret = rcutils_steady_time_now(&now);
  });
  std::chrono::steady_clock::time_point now_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  // Wait for a little while.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  // Then take a new timestamp with each and compare.
  rcutils_time_point_value_t later;
  EXPECT_NO_MEMORY_OPERATIONS({
    ret = rcutils_steady_time_now(&later);
  });
  std::chrono::steady_clock::time_point later_sc = std::chrono::steady_clock::now();
  EXPECT_EQ(ret, RCUTILS_RET_OK) << rcutils_get_error_string_safe();
  int64_t steady_diff = later - now;
  int64_t sc_diff =
    std::chrono::duration_cast<std::chrono::nanoseconds>(later_sc - now_sc).count();
  const int k_tolerance_ms = 1;
  EXPECT_LE(
    llabs(steady_diff - sc_diff), RCUTILS_MS_TO_NS(k_tolerance_ms)) << "steady_clock differs";
}
