// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_DWARF_INFO_PARSER_H_
#define DWARF_READER_DWARF_INFO_PARSER_H_

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
class DwarfInfoParser : public dwarf2reader::Dwarf2Handler {
 public:
  DwarfInfoParser(ParseState *parse_state, IDwarfReader *reader);

  // Start to process a compilation unit at OFFSET from the beginning of the
  // .debug_info section. Return false if you would like to skip this
  // compilation unit.
  virtual bool StartCompilationUnit(uint64 offset,
                                    uint8 address_size,
                                    uint8 offset_size,
                                    uint64 compilation_unit_length,
                                    uint8 dwarf_version);

  virtual bool StartDIE(uint64 offset,
                        enum dwarf2reader::DwarfTag tag,
                        const dwarf2reader::AttributeList& attributes);

  virtual void EndDIE(uint64 offset);

  virtual void ProcessAttributeUnsigned(
      uint64 offset,
      enum dwarf2reader::DwarfAttribute attribute,
      enum dwarf2reader::DwarfForm form, uint64 data);

  virtual void ProcessAttributeSigned(
      uint64 offset,
      enum dwarf2reader::DwarfAttribute attribute,
      enum dwarf2reader::DwarfForm form,
      int64 data);

  virtual void ProcessAttributeReference(
      uint64 offset,
      enum dwarf2reader::DwarfAttribute attribute,
      enum dwarf2reader::DwarfForm form,
      uint64 data);

  virtual void ProcessAttributeBuffer(
      uint64 offset,
      enum dwarf2reader::DwarfAttribute attribute,
      enum dwarf2reader::DwarfForm form,
      const char* data,
      uint64 len);

  virtual void ProcessAttributeString(
      uint64 offset,
      enum dwarf2reader::DwarfAttribute attribute,
      enum dwarf2reader::DwarfForm form,
      const std::string& data);

 private:
  ParseState *parse_state_;
  IDwarfReader *reader_;
};

}  // namespace dwarf_reader

#endif  // DWARF_READER_DWARF_INFO_PARSER_H_
