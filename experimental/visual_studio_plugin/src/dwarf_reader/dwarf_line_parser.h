// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_DWARF_LINE_PARSER_H_
#define DWARF_READER_DWARF_LINE_PARSER_H_

#include <string>

#include "common/dwarf/dwarf2reader.h"

namespace dwarf_reader {

class IDwarfReader;
class ParseState;

/// The type this class inherits from is defined in breakpad.  The functions
/// in this class get called as breakpad parses the DWARF information from the
/// binary file that is being debugged.  This particular implementation fowards
/// the calls to a DWARF reader class, which can be written separately.
/// For detailed documentation on each function's expected behavior, please see
/// breakpad\dwarf2reader.h
class DwarfLineParser : public dwarf2reader::LineInfoHandler {
 public:
  DwarfLineParser(ParseState *parse_state, IDwarfReader *reader);

  virtual void DefineDir(const std::string& name, uint32 dir_num);

  virtual void DefineFile(const std::string& name,
                          int32 file_num,
                          uint32 dir_num,
                          uint64 mod_time,
                          uint64 length);

  virtual void AddLine(uint64 address,
                       uint64 length,
                       uint32 file_num,
                       uint32 line_num,
                       uint32 column_num);

 private:
  ParseState *parse_state_;
  IDwarfReader *reader_;
};

}  // namespace dwarf_parser

#endif  // DWARF_READER_DWARF_LINE_PARSER_H_
