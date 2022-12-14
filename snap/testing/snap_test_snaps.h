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

#ifndef THIRD_PARTY_SILIFUZZ_SNAP_TESTING_SNAP_TEST_SNAPS_H_
#define THIRD_PARTY_SILIFUZZ_SNAP_TESTING_SNAP_TEST_SNAPS_H_

#include <cstddef>

#include "./common/snapshot_test_enum.h"
#include "./snap/snap.h"
#include "./snap/testing/snap_test_types.h"
#include "./util/checks.h"
#include "./util/itoa.h"

namespace silifuzz {

// Generated by snap_test_snaps_gen for test types defined in snap_test_types.
extern const SnapCorpus kSnapGeneratorTestCorpus;
extern const SnapCorpus kSnapRunnerTestCorpus;

// Returns a Snap generator test Snap of the given type.
inline const Snap& GetSnapGeneratorTestSnap(SnapGeneratorTestType type) {
  const Snap* snap = kSnapGeneratorTestCorpus.Find(EnumStr(type));
  if (snap == nullptr) {
    LOG_FATAL("Cannot find snap with ID: ", EnumStr(type));
  }
  return *snap;
}

// Returns a Snap runner test Snap of the given type.
inline const Snap& GetSnapRunnerTestSnap(TestSnapshot type) {
  const Snap* snap = kSnapRunnerTestCorpus.Find(EnumStr(type));
  if (snap == nullptr) {
    LOG_FATAL("Cannot find snap with ID: ", EnumStr(type));
  }
  return *snap;
}

}  // namespace silifuzz

#endif  // THIRD_PARTY_SILIFUZZ_SNAP_TESTING_SNAP_TEST_SNAPS_H_
