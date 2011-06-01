// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CORE_DEBUG_BREAKPOINT_H_
#define DEBUGGER_CORE_DEBUG_BREAKPOINT_H_

namespace debug {
class IDebuggeeProcess;

/// This class represents a physical breakpoint.

/// Breakpoint is identified by flat address in debuggee process.
/// Breakpoint is implemented by writing 'int 3' (0xCC) in the specified
/// location in debuggee memory.
///
/// Class diagram (and more) is here:
/// https://docs.google.com/a/google.com/document/d/1lTN-IYqDd_oy9XQg9-zlNc_vbg-qyr4q2MKNEjhSA84/edit?hl=en&authkey=CJyJlOgF#
class Breakpoint {
 public:
  Breakpoint();
  Breakpoint(void* address, IDebuggeeProcess* process);
  ~Breakpoint() {}

  void* address() const;
  unsigned char original_code_byte() const;

  /// @return true iff |Init| operation succeeded.
  bool is_valid() const;

  /// Copies original byte at breakpoint address into original_code_byte_.
  /// Writes a breakpoint instruction code (0xCC) in memory position
  /// specified by breakpoint address.
  /// Should be called once.
  /// @return true if read and write succeeded.
  bool Init();

  /// Writes a breakpoint instruction code (0xCC) in memory position
  /// specified by breakpoint address.
  /// @return true if write succeeded.
  bool WriteBreakpointCode();

  /// Writes an original code in memory position specified
  /// by breakpoint address.
  /// @return true if write succeeded.
  bool RecoverCodeAtBreakpoint();

 private:
  void* address_;
  unsigned char original_code_byte_;
  bool is_valid_;
  IDebuggeeProcess* process_;

  Breakpoint(const Breakpoint&);  // DISALLOW_COPY_AND_ASSIGN
  void operator=(const Breakpoint&);
};
}  // namespace debug
#endif  // DEBUGGER_CORE_DEBUG_BREAKPOINT_H_

