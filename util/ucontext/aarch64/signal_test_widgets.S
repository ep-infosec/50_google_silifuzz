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

  .text

  .p2align 2
  .globl  UnmappedRead
  .type   UnmappedRead, @function
UnmappedRead:
  ldr x0, [x0]
  ret
  .size   UnmappedRead, .-UnmappedRead

  .p2align 2
  .globl  UnmappedWrite
  .type   UnmappedWrite, @function
UnmappedWrite:
  str x0, [x0]
  ret
  .size   UnmappedWrite, .-UnmappedWrite

  .p2align 2
  .globl  IllegalInstruction
  .type   IllegalInstruction, @function
IllegalInstruction:
  udf 0
  ret
  .size   IllegalInstruction, .-IllegalInstruction

  .p2align 2
  .globl  PrivilegedInstruction
  .type   PrivilegedInstruction, @function
PrivilegedInstruction:
  mrs x0, esr_el1
  ret
  .size   PrivilegedInstruction, .-PrivilegedInstruction

  .p2align 2
  .globl  DebugInstruction
  .type   DebugInstruction, @function
DebugInstruction:
  brk #7
  ret
  .size   DebugInstruction, .-DebugInstruction

  .p2align 2
  .globl  UnalignedStack
  .type   UnalignedStack, @function
UnalignedStack:
  str x0, [sp, #-8]!
  ldr x0, [sp], #8
  ret
  .size   UnalignedStack, .-UnalignedStack
