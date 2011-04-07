// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/core/debuggee_breakpoint_test.h"
#include <stdio.h>
#include "debugger/core/debuggee_breakpoint.h"
#include "debugger/core/debuggee_process.h"

namespace {
int report_error(const char* error, const char* file, int line) {
  printf("Test assert(%s) at %s.%d\n", error, file, line);
  return line;
}
}

#define my_assert(x) \
do {\
  if (!(x)) return report_error(#x, __FILE__, __LINE__);\
} while (false)

namespace {
const unsigned char kBreakpontCode = 0xCC;
const unsigned char kFillChar = 0xA7;
void* kBigAddr = reinterpret_cast<void*>(0xAAFFEE88);
void* kSmallAddr = reinterpret_cast<void*>(2);

// Mock DebuggeeProcess, used in DebuggeeBreakpoint unit tests.
// Provides ReadMemory and WriteMemory methods that access internal buffer,
// not debuggee process memory. Only single byte reads/writes are
// supported - breakpoints are one byte instruction.

class DebuggeeProcessMock : public debug::DebuggeeProcess {
 public:
  DebuggeeProcessMock() : debug::DebuggeeProcess(0, NULL, 0, NULL, NULL) {
    memset(buff, kFillChar, sizeof(buff));
  }

  virtual bool ReadMemory(const void* addr, size_t size, void* destination) {
    intptr_t offset = reinterpret_cast<int>(addr);
    if ((offset < sizeof(buff)) && (1 == size)) {
      memcpy(destination, &buff[offset], size);
      return true;
    }
    return false;
  }

  virtual bool WriteMemory(const void* addr, const void* source, size_t size) {
    intptr_t offset = reinterpret_cast<int>(addr);
    if ((offset < sizeof(buff)) && (1 == size)) {
      memcpy(&buff[offset], source, size);
      return true;
    }
    return false;
  }

  char buff[1000];
};
}  // namespace

namespace debug {
int DebuggeeBreakpointTest::Run() {
  if (true) {
    // Tests empty, uninitialized breakpoints.
    debug::Breakpoint bp;
    my_assert(NULL == bp.address());
    my_assert(0 == bp.original_code_byte());
    my_assert(!bp.is_valid());
  }
  if (true) {
    // Tests uninitialized breakpoints.
    void* addr = kBigAddr;
    debug::Breakpoint bp(addr);
    my_assert(addr == bp.address());
    my_assert(0 == bp.original_code_byte());
    my_assert(!bp.is_valid());
  }

  if (true) {
    void* big_addr = kBigAddr;
    void* addr = kSmallAddr;
    DebuggeeProcessMock proc;
    debug::Breakpoint bp(addr);
    unsigned char orig_code = 0;

    // Make sure original code byte is correct.
    my_assert(proc.ReadMemory(addr, sizeof(orig_code), &orig_code));
    my_assert(kFillChar == orig_code);

    // Make sure ReadMemory fails when called with big address.
    my_assert(!proc.ReadMemory(big_addr, sizeof(orig_code), &orig_code));
    my_assert(kFillChar == orig_code);

    // Tests breakpoint initialization.
    my_assert(bp.Init(&proc));
    my_assert(kFillChar == bp.original_code_byte());
    my_assert(bp.is_valid());
    unsigned char code = 0;
    my_assert(proc.ReadMemory(addr, sizeof(code), &code));
    my_assert(kBreakpontCode == code);

    // Tests breakpoint removal.
    my_assert(bp.RecoverCodeAtBreakpoint(&proc));
    my_assert(proc.ReadMemory(addr, sizeof(code), &code));
    my_assert(kFillChar == code);

    // Tests breakpoint recovery.
    my_assert(bp.WriteBreakpointCode(&proc));
    my_assert(proc.ReadMemory(addr, sizeof(code), &code));
    my_assert(kBreakpontCode == code);

    // Tests breakpoint operations on invalid breakpoint.
    bp.Invalidate();
    my_assert(!bp.RecoverCodeAtBreakpoint(&proc));
    my_assert(!bp.WriteBreakpointCode(&proc));
    my_assert(bp.Init(&proc));
    my_assert(bp.is_valid());
  }

  // Test how code handles memory access problems.
  if (true) {
    DebuggeeProcessMock proc;
    debug::Breakpoint bp(kBigAddr);
    my_assert(!bp.Init(&proc));
    my_assert(!bp.is_valid());
  }

  return 0;  // Unit test passed.
}
}  // namespace debug

