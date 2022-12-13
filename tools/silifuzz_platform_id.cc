// Copyright 2022 The SiliFuzz Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>

#include "./util/itoa.h"
#include "./util/platform.h"

// A simple binary to output the current platform ID. Used by shell test(s).
int main() {
  auto platform_id = silifuzz::CurrentPlatformId();

  if (platform_id != silifuzz::PlatformId::kUndefined) {
    std::cout << silifuzz::EnumStr(platform_id) << std::endl;
  } else {
    std::cerr << "Unsupported platform" << std::endl;
    return 1;
  }
  return 0;
}
