// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file contains the interface for processing ELF Data

#ifndef ELF_READER_ELF_READER_H_
#define ELF_READER_ELF_READER_H_

#include "common/types.h"

namespace elf_reader {

// Pure interface class for an ElfReader.
class IElfReader {
 public:
  // Called at the beginning of processing, with basic information
  // about the ELF file being processed.
  virtual void Init(const char *path,
                    void *data,
                    uint64_t length,
                    uint32_t classSize,
                    bool lsb) = 0;

  // These functions are called when processing sections headers.  The
  // SectionHeaderStart function must return true for this data to
  // be processed.
  virtual bool SectionHeadersStart(uint32_t count) = 0;
  virtual void SectionHeadersEnd() = 0;
  virtual void AddSectionHeader(const char *name,
                                void *data,
                                uint64_t virt,
                                uint32_t type,
                                uint32_t flags,
                                uint64_t size) = 0;
};

}  // namespace elf_reader

#endif  // ELF_READER_ELF_READER_H_

