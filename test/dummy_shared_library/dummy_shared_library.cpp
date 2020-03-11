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

#ifndef DUMMY_SHARED_LIBRARY__DUMMY_SHARED_LIBRARY_BASE_HPP_
#define DUMMY_SHARED_LIBRARY__DUMMY_SHARED_LIBRARY_BASE_HPP_

#include <iostream>

class DummySharedLibraryBase
{
public:
  virtual ~DummySharedLibraryBase() {}
  virtual void speak() = 0;
};

class Bar : public DummySharedLibraryBase
{
public:
  virtual ~Bar() = default;
  void speak()
  {
    printf("from plugin Bar\n");
  }
};

class Baz : public DummySharedLibraryBase
{
public:
  virtual ~Baz() = default;
  void speak()
  {
    printf("from plugin Baz");
  }
};


#endif  // DUMMY_SHARED_LIBRARY__DUMMY_SHARED_LIBRARY_BASE_HPP_
