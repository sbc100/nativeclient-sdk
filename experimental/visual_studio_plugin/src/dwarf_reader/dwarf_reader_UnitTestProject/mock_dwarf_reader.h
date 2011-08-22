// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_DWARF_READER_UNITTESTPROJECT_MOCK_DWARF_READER_H_
#define DWARF_READER_DWARF_READER_UNITTESTPROJECT_MOCK_DWARF_READER_H_

#include "common/dwarf/dwarf2enums.h"
#include "dwarf_reader/dwarf_reader.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dwarf_reader_tests {

class MockDwarfReader : public dwarf_reader::IDwarfReader {
  public:
  MOCK_METHOD5(StartCompilationUnit, void *(uint64 offset,
                                            uint8 address_size,
                                            uint8 offset_size,
                                            uint64 cu_length,
                                            uint8 dwarf_version));

  MOCK_METHOD2(EndCompilationUnit, void(void *ctx, uint64 offset));

  MOCK_METHOD4(StartDIE, void *(void *ctx,
                                uint64 parent,
                                uint64 offset,
                                enum dwarf2reader::DwarfTag tag));

  MOCK_METHOD2(EndDIE, void(void *ctx, uint64 offset));

  MOCK_METHOD6(ProcessAttributeUnsigned,
               void(void *ctx,
                    uint64 offset,
                    uint64 parent,
                    enum dwarf2reader::DwarfAttribute attr,
                    enum dwarf2reader::DwarfForm form,
                    uint64 data));

  MOCK_METHOD6(ProcessAttributeSigned,
               void(void *ctx,
                    uint64 offset,
                    uint64 parent,
                    enum dwarf2reader::DwarfAttribute attr,
                    enum dwarf2reader::DwarfForm form,
                    int64 data));

  MOCK_METHOD6(ProcessAttributeReference,
               void(void *ctx,
                    uint64 offset,
                    uint64 parent,
                    enum dwarf2reader::DwarfAttribute attr,
                    enum dwarf2reader::DwarfForm form,
                    uint64 data));

  MOCK_METHOD7(ProcessAttributeBuffer,
               void(void *ctx,
                    uint64 offset,
                    uint64 parent,
                    enum dwarf2reader::DwarfAttribute attr,
                    enum dwarf2reader::DwarfForm form,
                    const char* data,
                    uint64 len));

  MOCK_METHOD6(ProcessAttributeString,
               void(void *ctx,
                    uint64 offset,
                    uint64 parent,
                    enum dwarf2reader::DwarfAttribute attr,
                    enum dwarf2reader::DwarfForm form,
                    const char* data));

  MOCK_METHOD3(DefineDir, void(void *ctx, const char *name, uint32 dir_num));

  MOCK_METHOD6(DefineFile, void(void *ctx,
                                const char *name,
                                int32 file_num,
                                uint32 dir_num,
                                uint64 mod_time,
                                uint64 length));

  MOCK_METHOD6(AddLine, void(void *ctx,
                             uint64 address,
                             uint64 length,
                             uint32 file_num,
                             uint32 line_num,
                             uint32 column_num));

  MOCK_METHOD6(AddLocListEntry, void(uint64 offset,
                                     bool is_first_entry,
                                     uint64 lowPc,
                                     uint64 highPc,
                                     const void* data,
                                     size_t dataSize));

  MOCK_METHOD6(BeginCfiEntry, bool(size_t offset,
                                   uint64 address,
                                   uint64 length,
                                   uint8 version,
                                   const char* augmentation,
                                   unsigned return_address));

  MOCK_METHOD7(AddCfiRule, bool(uint64 address,
                                int reg,
                                CFI_RuleType ruleType,
                                int base_register,
                                int32 offset,
                                const void* expression,
                                uint32 expressionLength));

  MOCK_METHOD0(EndCfiEntry, bool());

  MOCK_METHOD4(AddRangeListEntry, void(uint64 offset,
                                       uint64 base_address,
                                       uint64 low_pc,
                                       uint64 high_pc));
};

}  // namespace dwarf_reader_tests

#endif  // DWARF_READER_DWARF_READER_UNITTESTPROJECT_MOCK_DWARF_READER_H_
