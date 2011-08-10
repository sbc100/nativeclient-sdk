// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file defines the ElfObject which decodes the elf data and calls
// the appropriate reader callbacks.  The reader interface allows the
// libary user to examine the ELF file without ever needing to know
// machine class (32/64 bit).

#ifndef DWARF_READER_DWARF_PARSE_H_
#define DWARF_READER_DWARF_PARSE_H_

#include "common/types.h"
#include "elf_reader/elf_object.h"

namespace dwarf_reader {

class IDwarfReader;
class IDwarfVM;

void DwarfParseElf(elf_reader::ElfObject *elf, IDwarfReader *reader);
uint64_t DwarfParseVM(IDwarfVM *vm, uint8_t *data, uint32_t length);

}  // namespace dwarf_reader

#endif  // DWARF_READER_DWARF_PARSE_H_

