// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>

#include "common/dwarf/bytereader.h"

#include "dwarf_reader/dwarf_frame_info_reader.h"
#include "dwarf_reader/dwarf_info_parser.h"
#include "dwarf_reader/dwarf_line_parser.h"
#include "dwarf_reader/dwarf_parser.h"
#include "dwarf_reader/dwarf_reader.h"
#include "dwarf_reader/elf_section_reader.h"
#include "dwarf_reader/parse_state.h"

#include "elf_reader/elf_object.h"

namespace {
  const uint64 kMaxValue = static_cast<uint64>(-1);
}

namespace dwarf_reader {

DwarfParser::DwarfParser()
    : elf_section_reader_(NULL),
      file_path_(NULL),
      is_initialized_(false) { }

DwarfParser::~DwarfParser() {
  delete elf_section_reader_;
}

bool DwarfParser::Init(elf_reader::ElfObject *elf_object) {
  if (NULL == elf_object) return false;

  file_path_ = elf_object->GetPath();
  elf_section_reader_ = new ElfSectionReader();

  if (NULL == file_path_ || NULL == elf_section_reader_) return false;

  elf_object->Parse(elf_section_reader_);
  is_initialized_ = true;

  return is_initialized_;
}

void DwarfParser::PopulateReader(IDwarfReader *dwarf_reader) const {
  if (is_initialized_) {
    PopulateCompilationUnits(dwarf_reader);
    PopulateCallFrameInfo(dwarf_reader);
    PopulateLocationLists(dwarf_reader);
    PopulateRangeLists(dwarf_reader);
  }
}

void DwarfParser::PopulateCallFrameInfo(IDwarfReader *dwarf_reader) const {
  SectionInfo debug_frame_section =
      elf_section_reader_->GetSectionInfo(".eh_frame");
  SectionInfo text_section = elf_section_reader_->GetSectionInfo(".text");

  elf_section_reader_->GetByteReader()->SetTextBase(
      reinterpret_cast<uint64>(text_section.first));
  elf_section_reader_->GetByteReader()->SetCFIDataBase(
      elf_section_reader_->GetSectionLoadAddress(".eh_frame"),
      debug_frame_section.first);

  // Read the call frame information
  dwarf2reader::CallFrameInfo::Reporter reporter(file_path_);
  DwarfFrameInfoReader handler(dwarf_reader);
  dwarf2reader::CallFrameInfo cfi_reader(
      debug_frame_section.first,
      debug_frame_section.second,
      elf_section_reader_->GetByteReader(),
      &handler,
      &reporter,
      true);
  cfi_reader.Start();
}

void DwarfParser::PopulateCompilationUnits(IDwarfReader *dwarf_reader) const {
  ParseState parse_state;

  SectionInfo debug_info_section =
      elf_section_reader_->GetSectionInfo(".debug_info");
  SectionInfo debug_line_section =
      elf_section_reader_->GetSectionInfo(".debug_line");

  DwarfInfoParser info_handler(&parse_state, dwarf_reader);
  DwarfLineParser line_handler(&parse_state, dwarf_reader);

  uint64 debug_info_length = debug_info_section.second;
  uint64 debug_line_length = debug_line_section.second;
  const char *debug_line_ptr = debug_line_section.first;
  for (uint64 offset = 0; offset < debug_info_length;) {
    dwarf2reader::CompilationUnit compilation_unit_reader(
        elf_section_reader_->sections(),
        offset,
        elf_section_reader_->GetByteReader(),
        &info_handler);

    // Process the entire compilation unit; get the offset of the next.
    offset += compilation_unit_reader.Start();

    // Process the matching line information; get the offset of the next.
    dwarf2reader::LineInfo lineInfo(debug_line_ptr,
                                    debug_line_length,
                                    elf_section_reader_->GetByteReader(),
                                    &line_handler);

    debug_line_ptr += lineInfo.Start();

    // Pop the end of the compilation unit manually
    dwarf_reader->EndCompilationUnit(parse_state.GetTopStackContext(),
                                     parse_state.GetTopStackAddress());
    parse_state.PopStackFrame();
  }
}

void DwarfParser::PopulateLocationLists(IDwarfReader *dwarf_reader) const {
  SectionInfo debug_loc_section =
      elf_section_reader_->GetSectionInfo(".debug_loc");

  // Read the location list
  const char* debug_loc_ptr = debug_loc_section.first;
  const char* current = debug_loc_ptr;
  uint64 debug_loc_length = debug_loc_section.second;
  const char* debug_loc_end = debug_loc_ptr + debug_loc_length;
  dwarf2reader::ByteReader* byte_reader =
      elf_section_reader_->GetByteReader();
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
    } else if (kMaxValue == low_pc) {
      // the location is an absolute address; its value is in high_pc.
      data_size = 4;
      data = &high_pc;
    } else {
      data_size = byte_reader->ReadTwoBytes(current);
      current += 2;
      data = reinterpret_cast<const void *>(current);
      current += data_size;
    }
    dwarf_reader->AddLocListEntry(offset,
                                 is_first,
                                 low_pc,
                                 high_pc,
                                 data,
                                 data_size);
    is_first = false;
  }
}

void DwarfParser::PopulateRangeLists(IDwarfReader *dwarf_reader) const {
  SectionInfo debug_ranges_section =
      elf_section_reader_->GetSectionInfo(".debug_ranges");

  const char *debug_ranges_start = debug_ranges_section.first;
  const char *current = debug_ranges_section.first;
  const char *debug_ranges_end =
      debug_ranges_start + debug_ranges_section.second;

  dwarf2reader::ByteReader *byte_reader =
      elf_section_reader_->GetByteReader();

  uint64 offset = 0;
  // When there is no explicit base_address set, we want to populate the range
  // list entry with max value.  By convention, this means that the
  // compilation unit's base will serve as the base for these entries.
  uint64 base_address = kMaxValue;

  while (current < debug_ranges_end) {
    uint64 low_pc = byte_reader->ReadAddress(current);
    current += byte_reader->AddressSize();
    uint64 high_pc = byte_reader->ReadAddress(current);
    current += byte_reader->AddressSize();

    if (0 == low_pc && 0 == high_pc) {
      // We are now looking at the start of the next list.
      offset = current - debug_ranges_start;
      base_address = kMaxValue;
    } else if (kMaxValue == low_pc) {
      // This entry is an base address; its value is in high_pc.
      base_address = high_pc;
    } else {
      dwarf_reader->AddRangeListEntry(offset, base_address, low_pc, high_pc);
    }
  }
}

}  // namespace dwarf_parser
