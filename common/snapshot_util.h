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

#ifndef THIRD_PARTY_SILIFUZZ_COMMON_SNAPSHOT_UTIL_H_
#define THIRD_PARTY_SILIFUZZ_COMMON_SNAPSHOT_UTIL_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "./common/snapshot.h"
#include "./util/ucontext/ucontext_types.h"

namespace silifuzz {

// Writes `snapshot` to `filename` as a binary proto.Snapshot.
absl::Status WriteSnapshotToFile(
    const Snapshot& snapshot, absl::string_view filename) ABSL_MUST_USE_RESULT;
void WriteSnapshotToFileOrDie(const Snapshot& snapshot,
                              absl::string_view filename);

// Reads Snapshot from `filename` (must be a binary proto.Snapshot).
absl::StatusOr<Snapshot> ReadSnapshotFromFile(absl::string_view filename)
    ABSL_MUST_USE_RESULT;
Snapshot ReadSnapshotFromFileOrDie(absl::string_view filename);

// Register data conversion helper.
// Returned RegisterState will be for Snapshot::CurrentArchitecture().
template <typename Arch>
Snapshot::RegisterState ConvertRegsToSnapshot(const GRegSet<Arch>& gregs,
                                              const FPRegSet<Arch>& fpregs);

// Register data conversion helper.
// REQUIRES: register_state is in Snapshot::CurrentArchitecture().
template <typename Arch>
absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<Arch>* gregs);
template <typename Arch>
absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<Arch>* gregs,
    FPRegSet<Arch>* fpregs);

}  // namespace silifuzz

#endif  // THIRD_PARTY_SILIFUZZ_COMMON_SNAPSHOT_UTIL_H_
