// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwarf_reader/dwarf_parser.h"
#include "dwarf_reader/dwarf_reader_UnitTestProject/base_fixture.h"
#include "dwarf_reader/dwarf_reader_UnitTestProject/mock_dwarf_reader.h"

#include "gtest/gtest.h"

namespace {
/// This fixture provides the context for testing the |DwarfParser|.
class DwarfParserTestFixture
    : public dwarf_reader_tests::BaseFixture {
 protected:
  virtual ~DwarfParserTestFixture() {
    delete dwarf_parser_;
  };

  /// Attempts to initialize |dwarf_parser_|
  /// @return False if it cannot.
  bool InitializeDwarfParser() {
    bool success = false;
    if (InitializeElfObject()) {
      dwarf_parser_ = new dwarf_reader::DwarfParser();
      success = dwarf_parser_->Init(&elf_object_);
    }
    return success;
  }

  dwarf_reader::DwarfParser *dwarf_parser_;
};

TEST_F(DwarfParserTestFixture, IsDwarfParserInitialized) {
  ASSERT_TRUE(InitializeDwarfParser());
}

TEST_F(DwarfParserTestFixture, TestPopulateReader) {
  using ::testing::_;
  using ::testing::AtLeast;

  InitializeDwarfParser();
  dwarf_reader_tests::MockDwarfReader mock_dwarf_reader;

  // This is a very vague test at the moment, but note that the specifics of
  // data validation necessarily have to be performed while testing the
  // SymbolDatabase and SymbolProvider at higher levels of abstraction.  Also,
  // removing any of these lines, will show each entry being added in the test
  // output, which definitely comes in handy.
  EXPECT_CALL(mock_dwarf_reader, StartCompilationUnit(_, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, EndCompilationUnit(_, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, StartDIE(_, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, EndDIE(_, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, ProcessAttributeUnsigned(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, ProcessAttributeReference(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, ProcessAttributeBuffer(_, _, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, ProcessAttributeString(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, DefineDir(_, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, DefineFile(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, AddLine(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, AddLocListEntry(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, BeginCfiEntry(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(mock_dwarf_reader, AddRangeListEntry(_, _, _, _))
      .Times(AtLeast(1));

  dwarf_parser_->PopulateReader(&mock_dwarf_reader);
}


}  // namespace
