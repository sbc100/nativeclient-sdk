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

namespace elf_reader {
class ElfObject;
}  // namespace elf_reader

namespace dwarf_reader {

class IDwarfReader;
class IDwarfVM;

/// Populates an IDwarfReader with debug information.
/// @param elf An ElfObject, which should be preloaded with a binary file
/// location.
/// @param reader An IDwarfReader which needs to be populated.
void DwarfParseElf(elf_reader::ElfObject *elf, IDwarfReader *reader);

/// Runs the IDwarfVM on the provided data.  |data| is expected to be a dwarf
/// expression in binary format of length |length|
/// @param dwarf_state_machine The state machine which must implement
/// IDwarfVM.
/// @param data The dwarf expression.
/// @param length The length of the dwarf expression.
/// @return The result of the expression.
uint64_t DwarfParseVM(IDwarfVM *dwarf_state_machine,
                      uint8_t *data,
                      uint32_t length);

}  // namespace dwarf_reader

#endif  // DWARF_READER_DWARF_PARSE_H_

