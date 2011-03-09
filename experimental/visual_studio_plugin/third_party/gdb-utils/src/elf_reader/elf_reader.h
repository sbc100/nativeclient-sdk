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


// This file contains the interface for processing ELF Data

#ifndef ELF_READER_ELF_READER_H_
#define ELF_READER_ELF_READER_H_

#include "common/types.h"

namespace elf_reader {


// 
// Pure interface class for an ElfReader.
//
class IElfReader {
 public:
  // Called at the beginning of processing, with basic information
  // about the ELF file being processed.
  virtual void Header(const char *path, void *data, uint64_t length,
                      uint32_t classSize, bool lsb) = 0;

  // These functions are called when processing sections headers.  The
  // SectionHeaderStart function must return true for this data to
  // be processed.
  virtual bool SectionHeadersStart(uint32_t count) = 0;
  virtual void SectionHeadersEnd() = 0;
  virtual void SectionHeader(const char *name, void *data, uint64_t virt,
                             uint32_t type, uint32_t flags, uint64_t size)=0;

  // These functions are called when processing program headers.  The
  // ProgramHeaderStart function must return true for this data to
  // be processed.
  virtual bool ProgramHeadersStart(uint32_t count) = 0;
  virtual void ProgramHeadersEnd() = 0;
  virtual void ProgramHeader(uint32_t type, uint32_t flags, uint64_t offset,
                             uint64_t virt, uint64_t fsize, uint64_t msize)=0;
};


// 
// Base class does nothing so that the user can choose to reimplement
// only those pieces which are needed
//
class ElfReaderBase : public IElfReader {
public:
  int foo();

  virtual ~ElfReaderBase() {};

  int bar();

  // Called at the beginning of processing, with basic information
  // about the ELF file being processed.
  void Header(const char *path, void *data, uint64_t length,
              uint32_t classSize, bool lsb) { };

  // These functions are called when processing sections headers.  The
  // SectionHeaderStart function must return true for this data to
  // be processed.
  bool SectionHeadersStart(uint32_t count) { return false; }
  void SectionHeadersEnd() {};
  void SectionHeader(const char *name, void *data, uint64_t virt,
                     uint32_t type, uint32_t flags, uint64_t size) {};

  // These functions are called when processing sections headers.  The
  // ProgramHeaderStart function must return true for this data to
  // be processed.
  bool ProgramHeadersStart(uint32_t count) { return false; }
  void ProgramHeadersEnd() { };
  void ProgramHeader(uint32_t type, uint32_t flags, uint64_t offset,
                     uint64_t virt, uint64_t fsize, uint64_t msize) {};

};

}  // namespace elf_reader

#endif  // ELF_READER_ELF_READER_H_

