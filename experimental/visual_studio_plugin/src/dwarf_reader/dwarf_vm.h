// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the interface for processing DWARF Virtual Machine

#ifndef DWARF_READER_DWARF_VM_H_
#define DWARF_READER_DWARF_VM_H_

#include "common/types.h"

namespace dwarf2reader {
class ByteReader;
}

namespace dwarf_reader {

// A virtual machine that operates on 32 bit addresses.
class IDwarfVM {

public:
  virtual uint32_t BitWidth() = 0;
  virtual bool IsLSB() = 0;
  virtual void ErrorString(const char *str) = 0;

  virtual uint64_t ReadRegister(int reg_number) = 0;
  virtual uint64_t ReadMemory(uint64_t address, int count) = 0;
  virtual uint64_t ReadFrameBase() = 0;
};

// Basic LSB implementations which can handle static structure decoding
class DwarfStaticVM32 : public IDwarfVM {
  uint32_t BitWidth() { return 32; }
  bool IsLSB() { return true; }
  void ErrorString(const char *str) {};

  uint64_t ReadRegister(int reg_number) { return 0; }
  uint64_t ReadMemory(uint64_t address, int count) { return 0; }
  uint64_t ReadFrameBase() { return 0; }
};

class DwarfStaticVM64 : public IDwarfVM {
  uint32_t BitWidth() { return 64; }
  bool IsLSB() { return true; }
  void ErrorString(const char *str) {};

  uint64_t ReadRegister(int reg_number) { return 0; }
  uint64_t ReadMemory(uint64_t address, int count) { return 0; }
  uint64_t ReadFrameBase() { return 0; }
};

uint32_t DwarfRun32(IDwarfVM *vm,
                    dwarf2reader::ByteReader *reader,
                    const char *program,
                    int length);
uint64_t DwarfRun64(IDwarfVM *vm,
                    dwarf2reader::ByteReader *reader,
                    const char *program,
                    int length);
}

#endif  // DWARF_READER_DWARF_VM_H_
