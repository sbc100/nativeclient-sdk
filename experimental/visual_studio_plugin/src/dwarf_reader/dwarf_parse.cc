// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/dwarf/bytereader.h"

#include "elf_reader/elf_object.h"

#include "dwarf_reader/dwarf_parser.h"
#include "dwarf_reader/dwarf_parse.h"
#include "dwarf_reader/dwarf_vm.h"


namespace dwarf_reader {

void DwarfParseElf(elf_reader::ElfObject *elf, IDwarfReader *reader) {
  DwarfParser dwarf_parser;
  if (dwarf_parser.Init(elf)) {
    dwarf_parser.PopulateReader(reader);
  }
}

uint64_t DwarfParseVM(IDwarfVM* dwarf_state_machine,
                      uint8_t *data,
                      uint32_t length) {
  uint64_t return_value = 0;
  dwarf2reader::ByteReader *byte_reader;
  if (dwarf_state_machine->IsLSB())
      byte_reader = new dwarf2reader::ByteReader(
          dwarf2reader::ENDIANNESS_LITTLE);
    else
      byte_reader = new dwarf2reader::ByteReader(
          dwarf2reader::ENDIANNESS_BIG);

  switch(dwarf_state_machine->BitWidth()) {
      case 32: 
        byte_reader->SetAddressSize(4);
        return_value = DwarfRun32(dwarf_state_machine,
                                  byte_reader,
                                  reinterpret_cast<const char *>(data),
                                  length);
        break;
      case 64:
        byte_reader->SetAddressSize(8);
        return_value = DwarfRun32(dwarf_state_machine,
                                  byte_reader,
                                  reinterpret_cast<const char *>(data),
                                  length);
        break;
  }
  delete byte_reader;
  return return_value;
}

}  // namespace dwarf_reader

