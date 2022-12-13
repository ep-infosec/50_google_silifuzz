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

#include "./common/snapshot_util.h"

#include "absl/status/statusor.h"
#include "./common/snapshot_proto.h"
#include "./util/checks.h"
#include "./util/proto_util.h"
#include "./util/ucontext/serialize.h"

namespace silifuzz {

absl::Status WriteSnapshotToFile(const Snapshot& snapshot,
                                 absl::string_view filename) {
  proto::Snapshot proto;
  SnapshotProto::ToProto(snapshot, &proto);
  return WriteToFile(proto, filename);
}

void WriteSnapshotToFileOrDie(const Snapshot& snapshot,
                              absl::string_view filename) {
  auto s = WriteSnapshotToFile(snapshot, filename);
  CHECK_STATUS(s);
}

absl::StatusOr<Snapshot> ReadSnapshotFromFile(absl::string_view filename) {
  proto::Snapshot snap_proto;
  auto s = ReadFromFile(filename, &snap_proto);
  RETURN_IF_NOT_OK(s);

  auto snapshot_or = SnapshotProto::FromProto(snap_proto);
  RETURN_IF_NOT_OK_PLUS(snapshot_or.status(),
                        "Could not parse Snapshot from proto: ");
  return snapshot_or;
}

Snapshot ReadSnapshotFromFileOrDie(absl::string_view filename) {
  auto snapshot_or = ReadSnapshotFromFile(filename);
  CHECK_STATUS(snapshot_or.status());
  return std::move(snapshot_or).value();
}

template <typename Arch>
Snapshot::RegisterState ConvertRegsToSnapshot(const GRegSet<Arch>& gregs,
                                              const FPRegSet<Arch>& fpregs) {
  Snapshot::ByteData gregs_bytes, fpregs_bytes;
  CHECK(SerializeGRegs(gregs, &gregs_bytes));
  CHECK(SerializeFPRegs(fpregs, &fpregs_bytes));
  return Snapshot::RegisterState(gregs_bytes, fpregs_bytes);
}

template Snapshot::RegisterState ConvertRegsToSnapshot(
    const GRegSet<X86_64>& gregs, const FPRegSet<X86_64>& fpregs);
template Snapshot::RegisterState ConvertRegsToSnapshot(
    const GRegSet<AArch64>& gregs, const FPRegSet<AArch64>& fpregs);

template <typename Arch>
absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<Arch>* gregs) {
  if (!DeserializeGRegs(register_state.gregs(), gregs)) {
    return absl::InvalidArgumentError("Failed to deserialize gregs");
  }
  return absl::OkStatus();
}

template absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<X86_64>* gregs);
template absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<AArch64>* gregs);

template <typename Arch>
absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<Arch>* gregs,
    FPRegSet<Arch>* fpregs) {
  RETURN_IF_NOT_OK(ConvertRegsFromSnapshot(register_state, gregs));
  if (!DeserializeFPRegs(register_state.fpregs(), fpregs)) {
    return absl::InvalidArgumentError("Failed to deserialize fpregs");
  }
  return absl::OkStatus();
}

template absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<X86_64>* gregs,
    FPRegSet<X86_64>* fpregs);
template absl::Status ConvertRegsFromSnapshot(
    const Snapshot::RegisterState& register_state, GRegSet<AArch64>* gregs,
    FPRegSet<AArch64>* fpregs);

}  // namespace silifuzz
