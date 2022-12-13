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

#include "./common/snapshot_test_config.h"
#include "./util/checks.h"

namespace silifuzz {
namespace {

// This file is generated by generate_tests.py. Do not edit this file by hand.

const TestSnapshotConfig configs[35] = {
    {
        .type = TestSnapshot::kEmpty,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12345000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12346000,
        .data_num_bytes = 0x1000,
        //     12345000:   <empty>
        .instruction_bytes = {},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kEndsAsExpected,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12355000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12356000,
        .data_num_bytes = 0x1000,
        //     12355000:   90                      nop
        .instruction_bytes = {0x90},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kEndsUnexpectedly,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12365000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12366000,
        .data_num_bytes = 0x1000,
        //     12365000:   cc                      int3
        .instruction_bytes = {0xcc},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kRegsMismatch,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12375000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12376000,
        .data_num_bytes = 0x1000,
        //     12375000:   48 31 e0                xor    %rsp,%rax
        .instruction_bytes = {0x48, 0x31, 0xe0},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kMemoryMismatch,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12385000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12386000,
        .data_num_bytes = 0x1000,
        //     12385000:   9c                      pushf
        //     12385001:   50                      push   %rax
        //     12385002:   48 31 c0                xor    %rax,%rax
        //     12385005:   48 f7 d0                not    %rax
        //     12385008:   50                      push   %rax
        //     12385009:   58                      pop    %rax
        //     1238500a:   58                      pop    %rax
        //     1238500b:   9d                      popf
        .instruction_bytes = {0x9c, 0x50, 0x48, 0x31, 0xc0, 0x48, 0xf7, 0xd0,
                              0x50, 0x58, 0x58, 0x9d},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kRegsAndMemoryMismatch,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12395000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12396000,
        .data_num_bytes = 0x1000,
        //     12395000:   48 31 e3                xor    %rsp,%rbx
        //     12395003:   48 31 c0                xor    %rax,%rax
        //     12395006:   48 f7 d0                not    %rax
        //     12395009:   50                      push   %rax
        .instruction_bytes = {0x48, 0x31, 0xe3, 0x48, 0x31, 0xc0, 0x48, 0xf7,
                              0xd0, 0x50},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kRegsMismatchRandom,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x123a5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123a6000,
        .data_num_bytes = 0x1000,
        //     123a5000:   48 c7 c0 01 00 00 00    mov    $0x1,%rax
        //     123a5007:   0f a2                   cpuid
        //     123a5009:   81 e1 00 00 00 40       and    $0x40000000,%ecx
        //     123a500f:   74 06                   je     0x123a5017
        //     123a5011:   48 0f c7 f0             rdrand %rax
        //     123a5015:   eb 02                   jmp    0x123a5019
        //     123a5017:   0f 31                   rdtsc
        .instruction_bytes = {0x48, 0xc7, 0xc0, 0x1,  0x0, 0x0,  0x0,
                              0xf,  0xa2, 0x81, 0xe1, 0x0, 0x0,  0x0,
                              0x40, 0x74, 0x6,  0x48, 0xf, 0xc7, 0xf0,
                              0xeb, 0x2,  0xf,  0x31},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kMemoryMismatchRandom,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x123b5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123b6000,
        .data_num_bytes = 0x1000,
        //     123b5000:   9c                      pushf
        //     123b5001:   50                      push   %rax
        //     123b5002:   52                      push   %rdx
        //     123b5003:   0f 31                   rdtsc
        //     123b5005:   50                      push   %rax
        //     123b5006:   58                      pop    %rax
        //     123b5007:   5a                      pop    %rdx
        //     123b5008:   58                      pop    %rax
        //     123b5009:   9d                      popf
        .instruction_bytes = {0x9c, 0x50, 0x52, 0xf, 0x31, 0x50, 0x58, 0x5a,
                              0x58, 0x9d},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kRegsAndMemoryMismatchRandom,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x123c5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123c6000,
        .data_num_bytes = 0x1000,
        //     123c5000:   0f 31                   rdtsc
        //     123c5002:   50                      push   %rax
        .instruction_bytes = {0xf, 0x31, 0x50},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kICEBP,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x123d5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123d6000,
        .data_num_bytes = 0x1000,
        //     123d5000:   f1                      int1
        .instruction_bytes = {0xf1},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kBreakpoint,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x123e5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123e6000,
        .data_num_bytes = 0x1000,
        //     123e5000:   cc                      int3
        .instruction_bytes = {0xcc},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kINT3_CD03,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x123f5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123f6000,
        .data_num_bytes = 0x1000,
        //     123f5000:   cd 03                   int    $0x3
        .instruction_bytes = {0xcd, 0x3},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSigIll,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12405000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12406000,
        .data_num_bytes = 0x1000,
        //     12405000:   0f 0b                   ud2
        .instruction_bytes = {0xf, 0xb},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSigSegvWrite,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12415000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12416000,
        .data_num_bytes = 0x1000,
        //     12415000:   48 8b 45 08             mov    0x8(%rbp),%rax
        //     12415004:   48 89 18                mov    %rbx,(%rax)
        .instruction_bytes = {0x48, 0x8b, 0x45, 0x8, 0x48, 0x89, 0x18},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSigSegvRead,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12425000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12426000,
        .data_num_bytes = 0x1000,
        //     12425000:   48 8b 45 00             mov    0x0(%rbp),%rax
        //     12425004:   48 8b 18                mov    (%rax),%rbx
        .instruction_bytes = {0x48, 0x8b, 0x45, 0x0, 0x48, 0x8b, 0x18},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSigSegvExec,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12435000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12436000,
        .data_num_bytes = 0x1000,
        //     12435000:   48 8b 45 10             mov    0x10(%rbp),%rax
        //     12435004:   ff e0                   jmp    *%rax
        .instruction_bytes = {0x48, 0x8b, 0x45, 0x10, 0xff, 0xe0},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSyscall,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12445000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12446000,
        .data_num_bytes = 0x1000,
        //     12445000:   48 31 f6                xor    %rsi,%rsi
        //     12445003:   48 31 ff                xor    %rdi,%rdi
        //     12445006:   48 c7 c0 35 01 00 00    mov    $0x135,%rax
        //     1244500d:   0f 05                   syscall
        //     1244500f:   48 31 c0                xor    %rax,%rax
        .instruction_bytes = {0x48, 0x31, 0xf6, 0x48, 0x31, 0xff, 0x48, 0xc7,
                              0xc0, 0x35, 0x1, 0x0, 0x0, 0xf, 0x5, 0x48, 0x31,
                              0xc0},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kGeneralProtectionFault,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12455000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12456000,
        .data_num_bytes = 0x1000,
        //     12455000:   0f ae 05 01 00 00 00    fxsave 0x1(%rip)        #
        //     0x12455008
        .instruction_bytes = {0xf, 0xae, 0x5, 0x1, 0x0, 0x0, 0x0},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kChangesSegmentReg,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12465000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12466000,
        .data_num_bytes = 0x1000,
        //     12465000:   66 b8 03 00             mov    $0x3,%ax
        //     12465004:   8e c0                   mov    %eax,%es
        //     12465006:   8e d8                   mov    %eax,%ds
        //     12465008:   8e e8                   mov    %eax,%gs
        //     1246500a:   8e e0                   mov    %eax,%fs
        .instruction_bytes = {0x66, 0xb8, 0x3, 0x0, 0x8e, 0xc0, 0x8e, 0xd8,
                              0x8e, 0xe8, 0x8e, 0xe0},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kIn,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12475000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12476000,
        .data_num_bytes = 0x1000,
        //     12475000:   ed                      in     (%dx),%eax
        .instruction_bytes = {0xed},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kRunaway,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12485000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12486000,
        .data_num_bytes = 0x1000,
        //     12485000:   eb fe                   jmp    0x12485000
        .instruction_bytes = {0xeb, 0xfe},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSplitLock,
        .arch = Snapshot::Architecture::kX86_64,
        .code_addr = 0x12495000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12496000,
        .data_num_bytes = 0x1000,
        //     12495000:   48 89 e0                mov    %rsp,%rax
        //     12495003:   48 ff c8                dec    %rax
        //     12495006:   30 c0                   xor    %al,%al
        //     12495008:   f0 ff 40 ff             lock incl -0x1(%rax)
        .instruction_bytes = {0x48, 0x89, 0xe0, 0x48, 0xff, 0xc8, 0x30, 0xc0,
                              0xf0, 0xff, 0x40, 0xff},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kEndsAsExpected,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x12345000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12346000,
        .data_num_bytes = 0x1000,
        //     12345000:   d503201f        nop
        .instruction_bytes = {0x1f, 0x20, 0x3, 0xd5},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kEndsUnexpectedly,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x12355000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12356000,
        .data_num_bytes = 0x1000,
        //     12355000:   00000000        udf     #0
        .instruction_bytes = {0x0, 0x0, 0x0, 0x0},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kRegsMismatch,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x12365000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12366000,
        .data_num_bytes = 0x1000,
        //     12365000:   aa2003e0        mvn     x0, x0
        .instruction_bytes = {0xe0, 0x3, 0x20, 0xaa},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kMemoryMismatch,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x12375000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12376000,
        .data_num_bytes = 0x1000,
        //     12375000:   f81f83e0        stur    x0, [sp, #-8]
        //     12375004:   aa3f03e0        mvn     x0, xzr
        //     12375008:   f81c03e0        stur    x0, [sp, #-64]
        //     1237500c:   f85f83e0        ldur    x0, [sp, #-8]
        .instruction_bytes = {0xe0, 0x83, 0x1f, 0xf8, 0xe0, 0x3, 0x3f, 0xaa,
                              0xe0, 0x3, 0x1c, 0xf8, 0xe0, 0x83, 0x5f, 0xf8},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kRegsMismatchRandom,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x12385000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12386000,
        .data_num_bytes = 0x1000,
        //     12385000:   d53be041        mrs     x1, cntvct_el0
        .instruction_bytes = {0x41, 0xe0, 0x3b, 0xd5},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kMemoryMismatchRandom,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x12395000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x12396000,
        .data_num_bytes = 0x1000,
        //     12395000:   f90003e1        str     x1, [sp]
        //     12395004:   d53be041        mrs     x1, cntvct_el0
        //     12395008:   f81c03e1        stur    x1, [sp, #-64]
        //     1239500c:   f94003e1        ldr     x1, [sp]
        .instruction_bytes = {0xe1, 0x3, 0x0, 0xf9, 0x41, 0xe0, 0x3b, 0xd5,
                              0xe1, 0x3, 0x1c, 0xf8, 0xe1, 0x3, 0x40, 0xf9},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kRegsAndMemoryMismatchRandom,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x123a5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123a6000,
        .data_num_bytes = 0x1000,
        //     123a5000:   d53be041        mrs     x1, cntvct_el0
        //     123a5004:   a9bf07e1        stp     x1, x1, [sp, #-16]!
        .instruction_bytes = {0x41, 0xe0, 0x3b, 0xd5, 0xe1, 0x7, 0xbf, 0xa9},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kBreakpoint,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x123b5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123b6000,
        .data_num_bytes = 0x1000,
        //     123b5000:   d4200000        brk     #0x0
        .instruction_bytes = {0x0, 0x0, 0x20, 0xd4},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSigSegvWrite,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x123c5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123c6000,
        .data_num_bytes = 0x1000,
        //     123c5000:   f94004c0        ldr     x0, [x6, #8]
        //     123c5004:   f9000001        str     x1, [x0]
        .instruction_bytes = {0xc0, 0x4, 0x40, 0xf9, 0x1, 0x0, 0x0, 0xf9},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSigSegvRead,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x123d5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123d6000,
        .data_num_bytes = 0x1000,
        //     123d5000:   f94000c0        ldr     x0, [x6]
        //     123d5004:   f9400000        ldr     x0, [x0]
        .instruction_bytes = {0xc0, 0x0, 0x40, 0xf9, 0x0, 0x0, 0x40, 0xf9},
        .normal_end = false,
    },
    {
        .type = TestSnapshot::kSyscall,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x123e5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123e6000,
        .data_num_bytes = 0x1000,
        //     123e5000:   aa1f03e0        mov     x0, xzr
        //     123e5004:   aa1f03e1        mov     x1, xzr
        //     123e5008:   aa1f03e2        mov     x2, xzr
        //     123e500c:   d28026a8        mov     x8, #0x135
        //     123e5010:   d4000001        svc     #0x0
        //     123e5014:   aa1f03e0        mov     x0, xzr
        .instruction_bytes = {0xe0, 0x3, 0x1f, 0xaa, 0xe1, 0x3,  0x1f, 0xaa,
                              0xe2, 0x3, 0x1f, 0xaa, 0xa8, 0x26, 0x80, 0xd2,
                              0x1,  0x0, 0x0,  0xd4, 0xe0, 0x3,  0x1f, 0xaa},
        .normal_end = true,
    },
    {
        .type = TestSnapshot::kRunaway,
        .arch = Snapshot::Architecture::kAArch64,
        .code_addr = 0x123f5000,
        .code_num_bytes = 0x1000,
        .data_addr = 0x123f6000,
        .data_num_bytes = 0x1000,
        //     123f5000:   14000000        b       0x123f5000
        .instruction_bytes = {0x0, 0x0, 0x0, 0x14},
        .normal_end = false,
    },
};

}  // namespace

const TestSnapshotConfig* GetTestSnapshotConfig(Snapshot::Architecture arch,
                                                TestSnapshot type) {
  for (size_t i = 0; i < sizeof(configs) / sizeof(TestSnapshotConfig); i++) {
    if (configs[i].arch == arch && configs[i].type == type) {
      return &configs[i];
    }
  }
  return nullptr;
}

}  // namespace silifuzz