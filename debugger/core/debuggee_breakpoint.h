// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUGGEE_BREAKPOINT_H_
#define DEBUGGER_CORE_DEBUGGEE_BREAKPOINT_H_
#include <windows.h>

namespace debug {
class DebuggeeProcess;

/// This class represents a physical breakpoint, identified by flat address
/// in debuggee process. Breakpoint is implemented by writing 'int 3' (0xCC)
/// in the specified location in debuggee memory.
///
/// Class diagram (and more) is here:
/// https://docs.google.com/a/google.com/document/d/1lTN-IYqDd_oy9XQg9-zlNc_vbg-qyr4q2MKNEjhSA84/edit?hl=en&authkey=CJyJlOgF#
class Breakpoint {
 public:
  Breakpoint();
  explicit Breakpoint(void* address);
  virtual ~Breakpoint() {}

  virtual void* address() const;
  virtual unsigned char original_code_byte() const;

  /// @return true iff |Init| operation succeeded.
  virtual bool is_valid() const;

  /// Copies original byte at breakpoint address into original_code_byte_.
  /// Writes a breakpoint instruction code (0xCC) in memory position
  /// specified by breakpoint address.
  /// @param process
  /// @return true if read and write succeeded.
  virtual bool Init(DebuggeeProcess* process);

  /// Writes a breakpoint instruction code (0xCC) in memory position
  /// specified by breakpoint address.
  /// @param process
  /// @return true if write succeeded.
  virtual bool WriteBreakpointCode(DebuggeeProcess* process);

  /// Writes an original code in memory position specified
  /// by breakpoint address.
  /// @param process
  /// @return true if write succeeded.
  virtual bool RecoverCodeAtBreakpoint(DebuggeeProcess* process);

  /// Invalidates a breakpont object, does not change debuggee memory.
  virtual void Invalidate();

 private:
  void* address_;
  unsigned char original_code_byte_;
  bool is_valid_;
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUGGEE_BREAKPOINT_H_
