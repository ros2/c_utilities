// Copyright 2016 Open Source Robotics Foundation, Inc.
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
#include <string>

#include "rcutils/logging_macros.h"
#include "rcutils/assert.h"


void test_assert(bool value)
{
  RCUTILS_ASSERT(value);
}


void test_assert_msg(bool value, std::string message)
{
  RCUTILS_ASSERT_MSG(value, "%s", message.c_str());
}


void test_break(void)
{
  RCUTILS_BREAK();
}

TEST(TestAssert, test_rcutils_assert)
{
  // Should pass
  test_assert(true);
  test_assert_msg(true, "no message");

  EXPECT_DEATH(test_assert(false), "");
  EXPECT_DEATH(test_assert_msg(false, "message output"), ".*message output.*");
}


TEST(TestAssert, test_rcutils_break)
{
  EXPECT_DEATH(test_break(), "");
}
