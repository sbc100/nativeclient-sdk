// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef DEBUGGER_CHROME_INTEGRATION_TEST_RSP_REGISTERS_H_
#define DEBUGGER_CHROME_INTEGRATION_TEST_RSP_REGISTERS_H_

#include <deque>
#include <string>
#include "debugger/base/debug_blob.h"

namespace debug {
struct RegisterDescription {
 public:
  int gdb_number_;  // Starts from zero.
  std::string gdb_name_;
  size_t gdb_offset_;
  size_t gdb_size_;
  size_t context_offset_;
  size_t context_size_;
};

class RegistersSet {
 public:
  void InitializeForWin32();
  void InitializeForWin64();

  bool ReadRegisterFromGdbBlob(const debug::Blob& blob,
                               const std::string& register_name,
                               uint64_t* destination) const;

  bool ReadRegisterFromGdbBlob(const debug::Blob& blob,
                               int register_number,
                               uint64_t* destination) const;

  bool WriteRegisterToGdbBlob(const std::string& register_name,
                              uint64_t value,
                              debug::Blob* destination_blob) const;

  bool WriteRegisterToGdbBlob(int register_number,
                              uint64_t value,
                              debug::Blob* destination_blob) const;

  bool HasRegister(const std::string& name);

 protected:
  void AddReg(int gdb_number,
              std::string gdb_name,
              int gdb_offset,
              int gdb_size,
              int context_offset,
              int context_size);
  const RegisterDescription* FindRegisterDescription(
      const std::string& register_name) const;

  bool ReadRegisterFromGdbBlob(const debug::Blob& blob,
                               const RegisterDescription& register_info,
                               uint64_t* destination) const;

  void WriteRegisterToGdbBlob(const RegisterDescription& register_info,
                              uint64_t value,
                              debug::Blob* destination_blob) const;

  std::deque<RegisterDescription> regs_;
};
}  // namespace debug

#endif  // DEBUGGER_CHROME_INTEGRATION_TEST_RSP_REGISTERS_H_

