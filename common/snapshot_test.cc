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

#include "./common/snapshot.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/status/status.h"
#include "./common/memory_perms.h"
#include "./common/snapshot_test_util.h"
#include "./common/snapshot_util.h"
#include "./util/platform.h"
#include "./util/testing/status_macros.h"
#include "./util/testing/status_matchers.h"
#include "./util/ucontext/ucontext.h"

namespace silifuzz {
namespace {

using silifuzz::testing::StatusIs;
using ::testing::ContainerEq;
using ::testing::ContainsRegex;
using ::testing::HasSubstr;
using ::testing::IsSupersetOf;
using ::testing::UnorderedElementsAre;

using arch_typelist = ::testing::Types<X86_64, AArch64>;
template <class>
struct SnapshotTest : ::testing::Test {};
TYPED_TEST_SUITE(SnapshotTest, arch_typelist);

// Returns a RegisterState with instruction and stack pointers set to the
// specified values. All other registers are set to 0 because the callers don't
// currently care.
template <typename Arch>
Snapshot::RegisterState CreateRegState(Snapshot::Address instruction_pointer,
                                       Snapshot::Address stack_pointer) {
  GRegSet<Arch> gregs;
  FPRegSet<Arch> fpregs;
  memset(&gregs, 0, sizeof(gregs));
  memset(&fpregs, 0, sizeof(fpregs));

  SetInstructionPointer(gregs, instruction_pointer);
  SetStackPointer(gregs, stack_pointer);

  return ConvertRegsToSnapshot(gregs, fpregs);
}

TYPED_TEST(SnapshotTest, Architecture) {
  EXPECT_EQ(Snapshot::Architecture::kX86_64,
            Snapshot::ArchitectureTypeToEnum<X86_64>());
  EXPECT_EQ(Snapshot::Architecture::kAArch64,
            Snapshot::ArchitectureTypeToEnum<AArch64>());
  EXPECT_EQ(Snapshot::CurrentArchitecture(),
            Snapshot::ArchitectureTypeToEnum<Host>());
}

TYPED_TEST(SnapshotTest, CanSetRegs) {
  Snapshot s(Snapshot::ArchitectureTypeToEnum<TypeParam>());
  s.add_memory_mapping(
      Snapshot::MemoryMapping::MakeSized(0x100000, 4096, MemoryPerms::All()));
  EXPECT_OK(s.can_set_registers(
      CreateRegState<TypeParam>(0x100000, 0x100000 + 4096)));

  auto status = s.can_set_registers(CreateRegState<TypeParam>(
      Snapshot::kUnsetRegisterValue, 0x100000 + 4096));
  EXPECT_THAT(status,
              StatusIs(absl::StatusCode::kInvalidArgument,
                       ContainsRegex("instruction pointer \\(0x.+\\) is not in "
                                     "an existing executable MemoryMapping")));

  status = s.can_set_registers(
      CreateRegState<TypeParam>(0x100000, Snapshot::kUnsetRegisterValue));
  EXPECT_THAT(
      status,
      StatusIs(absl::StatusCode::kInvalidArgument,
               ContainsRegex("stack pointer \\(0x.+\\) and [0-9]+ bytes before "
                             "it must be within a writable MemoryMapping")));
}

TYPED_TEST(SnapshotTest, IsComplete) {
  Snapshot s = CreateTestSnapshot<TypeParam>(TestSnapshot::kSigSegvRead);
  EXPECT_OK(s.IsCompleteSomeState());
  Snapshot::Endpoint ep(Snapshot::Endpoint::kSigSegv,
                        Snapshot::Endpoint::kSegvCantRead, 0x10000,
                        s.ExtractRip(s.registers()));
  Snapshot::EndState es(ep);
  ASSERT_OK(s.AddNegativeMemoryMappingsFor(es));

  EXPECT_THAT(s.IsCompleteSomeState(),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Unnecessary negative_memory_mappings")));

  ASSERT_OK(s.can_add_expected_end_state(es));
  s.add_expected_end_state(es);
  EXPECT_OK(s.IsCompleteSomeState());
  s.set_negative_memory_mappings({});
  EXPECT_THAT(s.IsCompleteSomeState(),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("Missing negative_memory_mappings")));
}

TYPED_TEST(SnapshotTest, EndStatePlatform) {
  Snapshot s = CreateTestSnapshot<TypeParam>(TestSnapshot::kEndsAsExpected);
  Snapshot::EndState es = s.expected_end_states()[0];
  ASSERT_FALSE(s.expected_end_states().empty());
  EXPECT_THAT(es.platforms(), UnorderedElementsAre(CurrentPlatformId()));
  es.add_platform(PlatformId::kIntelIcelake);
  es.add_platform(PlatformId::kIntelSapphireRapids);
  EXPECT_TRUE(es.has_platform(PlatformId::kIntelIcelake));
  EXPECT_TRUE(es.has_platform(PlatformId::kIntelSapphireRapids));
  EXPECT_FALSE(es.has_platform(PlatformId::kUndefined));
  EXPECT_FALSE(es.has_platform(PlatformId::kNonExistent));
  EXPECT_THAT(es.platforms(), IsSupersetOf({PlatformId::kIntelIcelake,
                                            PlatformId::kIntelSapphireRapids}));
  s.add_platforms_to_expected_end_state(0, es);
  EXPECT_THAT(s.expected_end_states()[0].platforms(),
              IsSupersetOf({PlatformId::kIntelIcelake,
                            PlatformId::kIntelSapphireRapids}));
}

TYPED_TEST(SnapshotTest, UndefinedPlatformAllowed) {
  Snapshot s = CreateTestSnapshot<TypeParam>(TestSnapshot::kEndsAsExpected);
  ASSERT_FALSE(s.expected_end_states().empty());
  Snapshot::EndState es = s.expected_end_states()[0];
  es.add_platform(PlatformId::kUndefined);
  EXPECT_TRUE(es.has_platform(PlatformId::kUndefined));
}

TYPED_TEST(SnapshotTest, NormalizeMemoryMappings) {
  Snapshot s(Snapshot::ArchitectureTypeToEnum<TypeParam>(), "id");
  constexpr Snapshot::Address kBase = 0x80000000;
  const size_t kPageSize = s.page_size();
  // Create 3 adjacent pages with different permissions.
  // Mappings are added not in ascending Address order to test sorting.
  const Snapshot::Address data_address = kBase + kPageSize * 2;
  const Snapshot::Address read_only_address = kBase + kPageSize;
  const Snapshot::Address code_address = kBase;

  const Snapshot::MemoryMapping data_mapping =
      Snapshot::MemoryMapping::MakeSized(data_address, kPageSize,
                                         MemoryPerms::RW());
  s.add_memory_mapping(data_mapping);

  const Snapshot::MemoryMapping read_only_mapping =
      Snapshot::MemoryMapping::MakeSized(read_only_address, kPageSize,
                                         MemoryPerms::R());
  s.add_memory_mapping(read_only_mapping);

  const Snapshot::MemoryMapping code_mapping =
      Snapshot::MemoryMapping::MakeSized(code_address, kPageSize,
                                         MemoryPerms::XR());
  s.add_memory_mapping(code_mapping);

  s.NormalizeMemoryMappings();

  Snapshot::MemoryMappingList expected{code_mapping, read_only_mapping,
                                       data_mapping};
  EXPECT_THAT(s.memory_mappings(), ContainerEq(expected));
}

TYPED_TEST(SnapshotTest, NormalizeMemoryBytesMerge) {
  Snapshot s(Snapshot::ArchitectureTypeToEnum<TypeParam>(), "id");
  constexpr Snapshot::Address kBase = 0x80000000;
  const size_t kPageSize = s.page_size();

  // Create two adjacent pages with different permissions.
  const Snapshot::MemoryMapping code_mapping =
      Snapshot::MemoryMapping::MakeSized(kBase, kPageSize, MemoryPerms::XR());
  s.add_memory_mapping(code_mapping);
  const Snapshot::MemoryMapping data_mapping =
      Snapshot::MemoryMapping::MakeSized(code_mapping.limit_address(),
                                         kPageSize, MemoryPerms::RW());
  s.add_memory_mapping(data_mapping);

  // Create two bytes that are adjacent but with different memory permissions.
  const Snapshot::MemoryBytes code_bytes(code_mapping.limit_address() - 1,
                                         {42});
  s.add_memory_bytes(code_bytes);
  const Snapshot::MemoryBytes data_bytes(code_bytes.limit_address(), {99});
  s.add_memory_bytes(data_bytes);

  const Snapshot::MemoryBytes data_bytes_2(data_bytes.limit_address(), {100});
  s.add_memory_bytes(data_bytes_2);

  /* This should not merge code bytes with data bytes but should merge the
   * data bytes above */
  s.NormalizeMemoryBytes();
  EXPECT_EQ(s.memory_bytes().size(), 2);
  EXPECT_EQ(s.memory_bytes()[0], code_bytes);
  EXPECT_EQ(s.memory_bytes()[1].start_address(), data_bytes.start_address());
  EXPECT_EQ(s.memory_bytes()[1].limit_address(), data_bytes_2.limit_address());
}

TYPED_TEST(SnapshotTest, NormalizeMemoryBytesSplit) {
  Snapshot s(Snapshot::ArchitectureTypeToEnum<TypeParam>(), "id");
  constexpr Snapshot::Address kBase = 0x80000000;
  const size_t kPageSize = s.page_size();

  // Create 3 adjacent pages with different permissions.
  // Mappings are added not in ascending Address order to test sorting.
  const Snapshot::Address data_address = kBase + kPageSize * 2;
  const Snapshot::Address read_only_address = kBase + kPageSize;
  const Snapshot::Address code_address = kBase;

  const Snapshot::MemoryMapping data_mapping =
      Snapshot::MemoryMapping::MakeSized(data_address, kPageSize,
                                         MemoryPerms::RW());
  s.add_memory_mapping(data_mapping);

  const Snapshot::MemoryMapping read_only_mapping =
      Snapshot::MemoryMapping::MakeSized(read_only_address, kPageSize,
                                         MemoryPerms::R());
  s.add_memory_mapping(read_only_mapping);

  const Snapshot::MemoryMapping code_mapping =
      Snapshot::MemoryMapping::MakeSized(code_address, kPageSize,
                                         MemoryPerms::XR());
  s.add_memory_mapping(code_mapping);

  // Create a single MemoryBytes that span across 3 pages.
  size_t mixed_perms_bytes_size =
      data_mapping.limit_address() - code_mapping.start_address();
  const Snapshot::MemoryBytes mixed_perms_bytes(
      code_mapping.start_address(),
      Snapshot::ByteData(mixed_perms_bytes_size, 0));
  s.add_memory_bytes(mixed_perms_bytes);

  // This should split the memory bytes above.
  s.NormalizeMemoryBytes();
  EXPECT_EQ(s.memory_bytes().size(), 3);
}

TYPED_TEST(SnapshotTest, ReplaceMemoryBytes) {
  Snapshot s = CreateTestSnapshot<TypeParam>(TestSnapshot::kEndsAsExpected);
  EXPECT_OK(s.IsComplete());
  EXPECT_TRUE(s.MappedMemoryIsDefined());

  {
    auto es = s.expected_end_states()[0];
    ASSERT_OK(s.IsComplete());
    const Snapshot::MemoryBytesList end_state_bytes = es.memory_bytes();
    ASSERT_THAT(es.ReplaceMemoryBytes({Snapshot::MemoryBytes(0x10000, {42}),
                                       Snapshot::MemoryBytes(0x10000, {42})}),
                StatusIs(absl::StatusCode::kInvalidArgument));
    ASSERT_OK(es.ReplaceMemoryBytes({}));
    EXPECT_EQ(es.changed_memory_set().byte_size(), 0);
    ASSERT_OK(
        es.ReplaceMemoryBytes(Snapshot::MemoryBytesList(end_state_bytes)));
    EXPECT_EQ(es.memory_bytes(), end_state_bytes);
    ASSERT_OK(s.IsComplete());
  }

  Snapshot::MemoryBytesList bytes = s.memory_bytes();

  ASSERT_OK(s.ReplaceMemoryBytes({}));
  EXPECT_FALSE(s.MappedMemoryIsDefined());

  ASSERT_OK(s.ReplaceMemoryBytes(std::move(bytes)));
  EXPECT_TRUE(s.MappedMemoryIsDefined());

  ASSERT_THAT(s.ReplaceMemoryBytes({Snapshot::MemoryBytes(0x10000, {42})}),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(MemoryBytes, Range) {
  Snapshot::MemoryBytes mb(42, "foobar");
  Snapshot::MemoryBytes left = mb.Range(42, 45);
  EXPECT_EQ(left.start_address(), 42);
  EXPECT_EQ(left.byte_values(), "foo");

  Snapshot::MemoryBytes right = mb.Range(45, 48);
  EXPECT_EQ(right.start_address(), 45);
  EXPECT_EQ(right.byte_values(), "bar");

  Snapshot::MemoryBytes middle = mb.Range(44, 46);
  EXPECT_EQ(middle.start_address(), 44);
  EXPECT_EQ(middle.byte_values(), "ob");
}

}  // namespace
}  // namespace silifuzz
