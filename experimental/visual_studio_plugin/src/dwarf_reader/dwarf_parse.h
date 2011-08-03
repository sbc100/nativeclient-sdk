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

