// Copyright 2010 Google, Inc.  All Rights reserved
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
 // met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


// This file contains the interface for processing DWARF Virtual Machine

#include "common/types.h"

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

}

