// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwarf_reader/dwarf_frame_info_reader.h"
#include "dwarf_reader/dwarf_reader.h"

namespace dwarf_reader {

DwarfFrameInfoReader::DwarfFrameInfoReader(IDwarfReader *reader)
  :  reader_(reader) {}

bool DwarfFrameInfoReader::Entry(size_t offset,
                                 uint64 address,
                                 uint64 length,
                                 uint8 version,
                                 const string &augmentation,
                                 unsigned return_address) {
    return reader_->BeginCfiEntry(offset,
                                  address,
                                  length,
                                  version,
                                  augmentation.c_str(),
                                  return_address);
}

bool DwarfFrameInfoReader::UndefinedRule(uint64 address, int reg) {
  return reader_->AddCfiRule(address,
                             reg,
                             IDwarfReader::CFIRT_UNDEFINED,
                             0,
                             0,
                             NULL,
                             0);
}

bool DwarfFrameInfoReader::SameValueRule(uint64 address, int reg) {
  return reader_->AddCfiRule(address,
                             reg,
                             IDwarfReader::CFIRT_SAMEVALUE,
                             0,
                             0,
                             NULL,
                             0);
}

bool DwarfFrameInfoReader::OffsetRule(uint64 address,
                                      int reg,
                                      int base_register,
                                      long offset) {
    return reader_->AddCfiRule(address,
                               reg,
                               IDwarfReader::CFIRT_OFFSET,
                               base_register,
                               offset,
                               NULL,
                               0);
}

bool DwarfFrameInfoReader::ValOffsetRule(uint64 address,
                                         int reg,
                                         int base_register,
                                         long offset) {
    return reader_->AddCfiRule(address,
                               reg,
                               IDwarfReader::CFIRT_VALOFFSET,
                               base_register,
                               offset,
                               NULL,
                               0);
}

bool DwarfFrameInfoReader::RegisterRule(uint64 address,
                                        int reg,
                                        int base_register) {
  return reader_->AddCfiRule(address,
                             reg,
                             IDwarfReader::CFIRT_REGISTER,
                             base_register,
                             0,
                             NULL,
                             0);
}

bool DwarfFrameInfoReader::ExpressionRule(uint64 address, int reg,
                                          const string &expression) {
    return reader_->AddCfiRule(address, reg, IDwarfReader::CFIRT_EXPRESSION,
                               0, 0, expression.c_str(),
                               expression.length());
}

bool DwarfFrameInfoReader::ValExpressionRule(uint64 address, int reg,
                                             const string &expression) {
    return reader_->AddCfiRule(address, reg,
                               IDwarfReader::CFIRT_VALEXPRESSION, 0, 0,
                               expression.c_str(),
                               expression.length());
}

bool DwarfFrameInfoReader::End() {
  return reader_->EndCfiEntry();
}

}  // namespace dwarf_reader
