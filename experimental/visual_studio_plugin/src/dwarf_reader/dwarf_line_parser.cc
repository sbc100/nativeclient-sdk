// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwarf_reader/dwarf_line_parser.h"
#include "dwarf_reader/dwarf_reader.h"
#include "dwarf_reader/parse_state.h"

namespace dwarf_reader {

DwarfLineParser::DwarfLineParser(ParseState *parse_state,
                                 IDwarfReader *reader)
  :  parse_state_(parse_state),
     reader_(reader) {}

void DwarfLineParser::DefineDir(const string& name, uint32 dir_num) {
  reader_->DefineDir(parse_state_->GetTopStackContext(),
                     name.data(),
                     dir_num);
}

void DwarfLineParser::DefineFile(const string& name,
                                 int32 file_num,
                                 uint32 dir_num,
                                 uint64 mod_time,
                                 uint64 length) {
  reader_->DefineFile(parse_state_->GetTopStackContext(),
                      name.data(),
                      file_num,
                      dir_num,
                      mod_time,
                      length);
}

void DwarfLineParser::AddLine(uint64 address,
                              uint64 length,
                              uint32 file_num,
                              uint32 line_num,
                              uint32 column_num) {
  reader_->AddLine(parse_state_->GetTopStackContext(),
                   address,
                   length,
                   file_num,
                   line_num,
                   column_num);
}

}  // namespace dwarf_reader
