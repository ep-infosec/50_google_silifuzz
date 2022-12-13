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

// A do-it-all tool for examining, manipulating, or creating
// snapshot proto files.

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "./common/snapshot.h"
#include "./common/snapshot_enums.h"
#include "./common/snapshot_printer.h"
#include "./common/snapshot_util.h"
#include "./runner/driver/runner_driver.h"
#include "./runner/runner_provider.h"
#include "./runner/snap_maker.h"
#include "./snap/gen/relocatable_snap_generator.h"
#include "./snap/gen/snap_generator.h"
#include "./util/checks.h"
#include "./util/enum_flag.h"
#include "./util/file_util.h"
#include "./util/platform.h"
#include "./util/tool_util.h"

namespace silifuzz {  // for ADL
DEFINE_ENUM_FLAG(SnapshotPrinter::RegsMode);
DEFINE_ENUM_FLAG(SnapshotPrinter::FPRegsMode);
DEFINE_ENUM_FLAG(SnapshotPrinter::EndRegsMode);
DEFINE_ENUM_FLAG(SnapshotPrinter::EndStateMode);
}  // namespace silifuzz

using silifuzz::SnapshotPrinter;

// Flags common to all commands that modify the snapshot:
ABSL_FLAG(bool, dry_run, false,
          "Whether commands that modify snapshot instead will only print it.");
ABSL_FLAG(bool, normalize, true,
          "Whether to also do Snapshot::NormalizeAll() of the output.");

// Flags that control `print` command (including in --dry_run mode):
ABSL_FLAG(SnapshotPrinter::RegsMode, regs, SnapshotPrinter::kNonZeroRegs,
          "Register printing mode. Values: all, non-0.");
ABSL_FLAG(SnapshotPrinter::FPRegsMode, fpregs, SnapshotPrinter::kAllFPRegs,
          "FP register printing mode. Values: all, ctrl, none.");
ABSL_FLAG(SnapshotPrinter::EndRegsMode, end_regs,
          SnapshotPrinter::kChangedEndRegs,
          "End state register printing mode. Values: all, changed.");
ABSL_FLAG(SnapshotPrinter::EndStateMode, end_states,
          SnapshotPrinter::kEndStateDiffs,
          "End state printing mode. Values: all, diffs.");
ABSL_FLAG(int64_t, bytes_limit, 200,
          "Limit on the number of bytes printed for one memory-bytes blob. "
          "-1 means no-limit.");
ABSL_FLAG(bool, stats, true, "Whether some summary stats are printed.");
ABSL_FLAG(bool, endpoints_only, false, "Whether to only print the endpoints.");
ABSL_FLAG(silifuzz::PlatformId, target_platform,
          silifuzz::PlatformId::kUndefined,
          "Target platform for commands like generate_corpus");

// ========================================================================= //

namespace silifuzz {

// Implements the `print` command.
void PrintSnapshot(const Snapshot& snapshot, LinePrinter* line_printer) {
  SnapshotPrinter::Options options;
  options.regs_mode = absl::GetFlag(FLAGS_regs);
  options.fp_regs_mode = absl::GetFlag(FLAGS_fpregs);
  options.end_state_regs_mode = absl::GetFlag(FLAGS_end_regs);
  options.end_state_mode = absl::GetFlag(FLAGS_end_states);
  options.bytes_limit = absl::GetFlag(FLAGS_bytes_limit);
  options.stats = absl::GetFlag(FLAGS_stats);

  SnapshotPrinter printer(line_printer, options);
  if (absl::GetFlag(FLAGS_endpoints_only)) {
    printer.PrintEndpointsOnly(snapshot);
  } else {
    printer.Print(snapshot);
  }
}

// Wrapper around WriteSnapshotToFileOrDie() that adds --normalize and
// --dry_run handling.
void OutputSnapshotOrDie(Snapshot&& snapshot, absl::string_view filename,
                         LinePrinter* line_printer) {
  if (absl::GetFlag(FLAGS_normalize)) snapshot.NormalizeAll();

  if (absl::GetFlag(FLAGS_dry_run)) {
    line_printer->Line("New snapshot state would be:");
    line_printer->Line("");
    PrintSnapshot(snapshot, line_printer);
  } else {
    CHECK_STATUS(snapshot.IsCompleteSomeState());
    WriteSnapshotToFileOrDie(snapshot, filename);
  }
}

// Implements `generate_corpus` command.
absl::Status GenerateCorpus(const std::vector<std::string>& input_protos,
                            PlatformId platform_id, LinePrinter* line_printer) {
  SnapifyOptions opts = SnapifyOptions::V2InputRunOpts();
  if (platform_id == PlatformId::kUndefined) {
    return absl::InvalidArgumentError(
        "generate_corpus requires a valid platform id");
  }
  opts.platform_id = platform_id;
  if (opts.platform_id == PlatformId::kUndefined) {
    return absl::InternalError("This platform is not supported by SiliFuzz");
  }

  std::vector<Snapshot> snapified_corpus;

  for (const std::string& proto_path : input_protos) {
    ASSIGN_OR_RETURN_IF_NOT_OK_PLUS(auto snapshot,
                                    ReadSnapshotFromFile(proto_path),
                                    "Cannot read snapshot");
    auto snapified_or = Snapify(snapshot, opts);
    if (!snapified_or.ok()) {
      line_printer->Line("Skipping ", proto_path, ": ",
                         snapified_or.status().message());
      continue;
    }
    snapified_corpus.push_back(std::move(snapified_or).value());
  }
  if (snapified_corpus.empty()) {
    return absl::InvalidArgumentError("No usable Snapshots found");
  }

  // TODO(ksteuck): Call PartitionSnapshots() to ensure there are no conflicts.

  RelocatableSnapGeneratorOptions options;
  MmappedMemoryPtr<char> buffer = GenerateRelocatableSnaps(
      PlatformArchitecture(platform_id), snapified_corpus, options);
  absl::string_view buf(buffer.get(), MmappedMemorySize(buffer));
  if (!WriteToFileDescriptor(STDOUT_FILENO, buf)) {
    return absl::InternalError("WriteToFileDescriptor failed");
  }
  return absl::OkStatus();
}

// ========================================================================= //

// Implements main().
// args are the non-flag arguments.
// Returns success status.
bool SnapToolMain(std::vector<char*>& args) {
  LinePrinter line_printer(LinePrinter::StdErrPrinter);

  std::string command;
  std::string snapshot_file;
  if (args.size() < 2) {
    line_printer.Line(
        "Expected one of {print,set_id,set_end,make,play,generate_corpus} and a"
        " snapshot file name(s).");
    return false;
  } else {
    command = ConsumeArg(args);
    snapshot_file = ConsumeArg(args);
  }

  Snapshot snapshot = ReadSnapshotFromFileOrDie(snapshot_file);

  if (command == "print") {
    if (ExtraArgs(args)) return false;

    PrintSnapshot(snapshot, &line_printer);
  } else if (command == "set_id") {
    if (args.size() != 1) {
      line_printer.Line("Expected one snapshot id value argument.");
      return false;
    }
    std::string id = ConsumeArg(args);

    if (absl::Status s = Snapshot::IsValidId(id); !s.ok()) {
      line_printer.Line("Invalid snapshot id value: ", s.ToString());
      return false;
    }

    snapshot.set_id(id);

    OutputSnapshotOrDie(std::move(snapshot), snapshot_file, &line_printer);
  } else if (command == "set_end") {
    // This is a way to manually replace snapshot's end-state(s) by one
    // specific endpoint address. It can be used to help one inspect
    // intermediate state of snapshot's execution.
    if (args.size() != 1) {
      line_printer.Line("Expected one endpoint hex address argument.");
      return false;
    }
    std::string addr = ConsumeArg(args);
    Snapshot::Address ep_addr;
    if (!absl::SimpleHexAtoi<Snapshot::Address>(addr, &ep_addr)) {
      line_printer.Line("Can't parse endpoint hex address: ", addr);
      return false;
    }

    Snapshot::Endpoint ep(ep_addr);
    Snapshot::EndState es(ep);
    auto s = snapshot.can_add_expected_end_state(es);
    if (!s.ok()) {
      line_printer.Line("Bad endpoint address value: ", addr, ": ",
                        s.message());
      return false;
    }
    snapshot.set_expected_end_states({es});

    OutputSnapshotOrDie(std::move(snapshot), snapshot_file, &line_printer);
  } else if (command == "play") {
    if (ExtraArgs(args)) return false;

    absl::StatusOr<RunnerDriver> runner_or =
        RunnerDriverFromSnapshot(snapshot, RunnerLocation());

    if (!runner_or.ok()) {
      line_printer.Line("Could not play snapshot: ",
                        runner_or.status().ToString());
    }
    auto result_or = runner_or->PlayOne(snapshot.id());
    if (!result_or.ok()) {
      line_printer.Line("Could not play snapshot: ",
                        result_or.status().ToString());
      return false;
    }
    if (result_or->success()) {
      line_printer.Line("Snapshot played successfully.");
      return true;
    } else {
      RunnerDriver::PlayerResult player_result = result_or->player_result();
      line_printer.Line("Snapshot played with outcome = ",
                        EnumStr(player_result.outcome));
      line_printer.Line("Actual end state reached:");
      SnapshotPrinter printer(&line_printer);
      printer.PrintActualEndState(snapshot,
                                  *result_or->player_result().actual_end_state);
      return false;
    }
  } else if (command == "make") {
    if (ExtraArgs(args)) return false;

    SnapMaker::Options opts;
    opts.runner_path = RunnerLocation();
    opts.num_verify_attempts = 1;
    SnapMaker maker(opts);

    absl::StatusOr<Snapshot> made_snapshot_or = maker.Make(snapshot);
    if (!made_snapshot_or.ok()) {
      line_printer.Line("Could not make snapshot: ",
                        made_snapshot_or.status().ToString());
      return false;
    }
    Snapshot made_snapshot = std::move(made_snapshot_or).value();

    absl::StatusOr<Snapshot> recorded_snapshot_or =
        maker.RecordEndState(made_snapshot);
    if (!recorded_snapshot_or.ok()) {
      line_printer.Line("Could not record snapshot: ",
                        recorded_snapshot_or.status().ToString());
      return false;
    }
    Snapshot recorded_snapshot = std::move(recorded_snapshot_or).value();

    DCHECK_EQ(recorded_snapshot.expected_end_states().size(), 1);
    const auto& ep = recorded_snapshot.expected_end_states()[0].endpoint();
    if (ep.type() != snapshot_types::Endpoint::kInstruction) {
      line_printer.Line("Cannot fix ", EnumStr(ep.sig_cause()), "/",
                        EnumStr(ep.sig_num()));
      return false;
    }
    absl::Status verify_status = maker.Verify(recorded_snapshot);
    if (!verify_status.ok()) {
      line_printer.Line(verify_status.message());
      return false;
    }
    line_printer.Line("Re-made snapshot succefully.");
    OutputSnapshotOrDie(std::move(recorded_snapshot), snapshot_file,
                        &line_printer);
  } else if (command == "generate_corpus") {
    std::vector<std::string> inputs({snapshot_file});
    for (const auto& a : args) {
      inputs.push_back(a);
    }
    absl::Status s = GenerateCorpus(
        inputs, absl::GetFlag(FLAGS_target_platform), &line_printer);
    if (!s.ok()) {
      line_printer.Line("Cannot generate corpus: ", s.message());
      return false;
    }
  } else {
    line_printer.Line("Unknown command is given: ", command);
    return false;
  }

  return true;
}

}  // namespace silifuzz

// ========================================================================= //

int main(int argc, char** argv) {
  std::string usage =
      absl::StrCat(argv[0], " command snapshot-file-name [command arg(s)]");
  std::vector<char*> positional_args = absl::ParseCommandLine(argc, argv);
  silifuzz::ConsumeArg(positional_args);  // skip binary's name
  bool success = silifuzz::SnapToolMain(positional_args);
  return silifuzz::ToExitCode(success);
}
