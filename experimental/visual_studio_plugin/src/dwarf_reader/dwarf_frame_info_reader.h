// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_DWARF_FRAME_INFO_READER_H_
#define DWARF_READER_DWARF_FRAME_INFO_READER_H_

#include <string>

#include "common\dwarf\dwarf2reader.h"


namespace dwarf_reader {

class IDwarfReader;

/// The type this class inherits from is defined in breakpad.  The functions
/// in this class get called as breakpad parses the DWARF information from the
/// binary file that is being debugged.  This particular implementation fowards
/// the calls to a DWARF reader class, which can be written separately.
/// For detailed documentation on each function's expected behavior, please see
/// breakpad\dwarf2reader.h
class DwarfFrameInfoReader : public dwarf2reader::CallFrameInfo::Handler {
 public:
  explicit DwarfFrameInfoReader(IDwarfReader *reader);

  virtual bool Entry(size_t offset,
                     uint64 address,
                     uint64 length,
                     uint8 version,
                     const string &augmentation,
                     unsigned return_address);

  virtual bool UndefinedRule(uint64 address, int reg);

  virtual bool SameValueRule(uint64 address, int reg);

  virtual bool OffsetRule(uint64 address,
                          int reg,
                          int base_register,
                          long offset);

  virtual bool ValOffsetRule(uint64 address,
                             int reg,
                             int base_register,
                             long offset);

  virtual bool RegisterRule(uint64 address, int reg, int base_register);

  virtual bool ExpressionRule(uint64 address,
                              int reg,
                              const std::string &expression);

  virtual bool ValExpressionRule(uint64 address,
                                 int reg,
                                 const std::string &expression);

  virtual bool End();

 private:
  IDwarfReader* reader_;
};

}  // namespace dwarf_reader

#endif  // DWARF_READER_ELF_SECTION_READER_H_
