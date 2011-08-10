// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stack>

#include "common/dwarf/bytereader.h"
#include "common/dwarf/dwarf2reader.h"
#include "common/types.h"

#include "elf_reader/elf_structs.h"
#include "elf_reader/elf_object.h"
#include "elf_reader/elf_reader.h"

#include "dwarf_reader/dwarf_frame_info_reader.h"
#include "dwarf_reader/dwarf_info_parser.h"
#include "dwarf_reader/dwarf_line_parser.h"
#include "dwarf_reader/dwarf_parse.h"
#include "dwarf_reader/dwarf_reader.h"
#include "dwarf_reader/dwarf_vm.h"
#include "dwarf_reader/elf_section_reader.h"
#include "dwarf_reader/parse_state.h"


using namespace elf_reader;
using namespace dwarf_reader;
using namespace dwarf2reader;

namespace dwarf_reader {

void DwarfParseElf(ElfObject *elf, IDwarfReader *reader) {
  ElfSectionReader *elf_info = new ElfSectionReader();
  ParseState parse_state;

  if (NULL == elf) return;
  if (NULL == reader) return;
  if (NULL == elf_info) return;

  elf->Parse(elf_info);
  SectionInfo debug_info_section = elf_info->GetSectionInfo(".debug_info");
  SectionInfo debug_line_section = elf_info->GetSectionInfo(".debug_line");
  SectionInfo debug_loc_section = elf_info->GetSectionInfo(".debug_loc");
  SectionInfo debug_frame_section = elf_info->GetSectionInfo(".eh_frame");
  SectionInfo text_section = elf_info->GetSectionInfo(".text");

  elf_info->GetByteReader()->SetTextBase(
      reinterpret_cast<uint64>(text_section.first));
  elf_info->GetByteReader()->SetCFIDataBase(
      elf_info->GetSectionLoadAddress(".eh_frame"),
      debug_frame_section.first);

  DwarfInfoParser info_handler(&parse_state, reader);
  DwarfLineParser line_handler(&parse_state, reader);

  uint64 debug_info_length = debug_info_section.second;
  uint64 debug_line_length = debug_line_section.second;
  const char *debug_line_ptr = debug_line_section.first;
  for (uint64 offset = 0; offset < debug_info_length;) {
    dwarf2reader::CompilationUnit compilation_unit_reader(
        elf_info->GetSectionMap(),
        offset,
        elf_info->GetByteReader(),
        &info_handler);

    // Process the entire compilation unit; get the offset of the next.
    offset += compilation_unit_reader.Start();

    // Process the matching line information; get the offset of the next.
    dwarf2reader::LineInfo lineInfo(debug_line_ptr,
                                    debug_line_length,
                                    elf_info->GetByteReader(),
                                    &line_handler);

    debug_line_ptr += lineInfo.Start();

    // Pop the end of the compilation unit manually
    reader->EndCompilationUnit(parse_state.GetTopStackContext(),
                               parse_state.GetTopStackAddress());
    parse_state.PopStackFrame();
  }

  // Read the call frame information
  dwarf2reader::CallFrameInfo::Reporter reporter(elf->GetPath());
  DwarfFrameInfoReader handler(reader);
  dwarf2reader::CallFrameInfo cfi_reader(debug_frame_section.first,
                                         debug_frame_section.second,
                                         elf_info->GetByteReader(),
                                         &handler,
                                         &reporter,
                                         true);
  cfi_reader.Start();

  // Read the location list
  // TODO(ilewis): this should probably move to a different function.
  const char* debug_loc_ptr = debug_loc_section.first;
  const char* current = debug_loc_ptr;
  uint64 debug_loc_length = debug_loc_section.second;
  const char* debug_loc_end = debug_loc_ptr + debug_loc_length;
  ByteReader* byte_reader = elf_info->GetByteReader();
  bool is_first = true;
  while (current < debug_loc_end) {
    // Layout of the debug_loc block is:
    //
    //  LowPc       - address
    //  HighPc      - address
    //  DataLength  - ushort (optional)
    //  Data        - byte[] (optional)
    uint64 offset = current - debug_loc_ptr;
    uint64 low_pc = byte_reader->ReadAddress(current);
    current += byte_reader->AddressSize();
    uint64 high_pc = byte_reader->ReadAddress(current);
    current += byte_reader->AddressSize();

    size_t data_size = 0;
    const void* data = 0;

    if (0 == low_pc && 0 == high_pc) {
      // if low_pc and high_pc are both zero, that signals end of list.
      is_first = true;
      continue;
    } else if (static_cast<uint64>(-1) == low_pc) {
      // the location is an absolute address; its value is in high_pc.
      data_size = 4;
      data = &high_pc;
    } else {
      data_size = byte_reader->ReadTwoBytes(current);
      current += 2;
      data = (void*)current;
      current += data_size;
    }
    reader->AddLocListEntry(offset,
                            is_first,
                            low_pc,
                            high_pc,
                            data,
                            data_size);
    is_first = false;
  }

  delete elf_info;
}


uint64_t DwarfParseVM(IDwarfVM* vm, uint8_t *data, uint32_t length) {
  ByteReader *byte_reader;
  if (vm->IsLSB())
      byte_reader = new ByteReader(ENDIANNESS_LITTLE);
    else
      byte_reader = new ByteReader(ENDIANNESS_BIG);
  switch(vm->BitWidth()) {
      case 32: 
        byte_reader->SetAddressSize(4);
        return DwarfRun32(vm,
                          byte_reader,
                          reinterpret_cast<const char *>(data),
                          length);
        break;
      case 64:
        byte_reader->SetAddressSize(8);
        return DwarfRun32(vm,
                          byte_reader,
                          reinterpret_cast<const char *>(data),
                          length);
        break;
  }
  return 0;
}
}  // namespace dwarf_reader

