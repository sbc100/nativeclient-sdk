// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <string>

#include "dwarf_reader/dwarf_reader_UnitTestProject/base_fixture.h"

#include "dwarf_reader/elf_section_reader.h"
#include "elf_reader/elf_object.h"
#include "gtest/gtest.h"

namespace {

/// This fixture provides the context for testing the |ElfSectionReader|.
class ElfSectionReaderFixture
    : public dwarf_reader_tests::BaseFixture {
 protected:
  virtual ~ElfSectionReaderFixture() {}

  /// Initializes the |section_reader_|.
  /// @return False if it can't.
  bool InitializeElfSectionReader() {
    bool success = false;
    if (InitializeElfObject()) {
      elf_object_.Parse(&section_reader_);
      success = !section_reader_.IsEmpty();
    }
    return success;
  }

  /// Checks if the reader has a given section.
  /// @return True, if it does; false if it doesnt.
  bool ReaderHasSection(const char * name) {
    dwarf_reader::SectionInfo info = section_reader_.GetSectionInfo(name);
    return (NULL != info.first);
  }

  dwarf_reader::ElfSectionReader section_reader_;
};

// Unit tests start here.
TEST_F(ElfSectionReaderFixture, CanElfObjectLoad) {
  ASSERT_TRUE(InitializeElfSectionReader());
}

TEST_F(ElfSectionReaderFixture, ContainsExpectedSections) {
  ASSERT_TRUE(InitializeElfSectionReader());

  // The following sections should be in the binary.
  std::vector<const char *> section_names;
  section_names.push_back(".init");
  section_names.push_back(".text");
  section_names.push_back(".fini");
  section_names.push_back(".rodata");
  section_names.push_back(".eh_frame_hdr");
  section_names.push_back(".eh_frame");
  section_names.push_back(".ctors");
  section_names.push_back(".dtors");
  section_names.push_back(".jcr");
  section_names.push_back(".got.plt");
  section_names.push_back(".data");
  section_names.push_back(".bss");
  section_names.push_back(".comment");
  section_names.push_back(".debug_aranges");
  section_names.push_back(".debug_pubnames");
  section_names.push_back(".debug_info");
  section_names.push_back(".debug_abbrev");
  section_names.push_back(".debug_line");
  section_names.push_back(".debug_str");
  section_names.push_back(".debug_loc");
  section_names.push_back(".debug_ranges");
  section_names.push_back(".shstrtab");
  section_names.push_back(".symtab");
  section_names.push_back(".strtab");

  bool all_sections_found = true;

  for (std::size_t i = 0; i < section_names.size(); ++i) {
    if (!ReaderHasSection(section_names[i])) {
      printf("Can't find an expected section in the binary: %s\n",
             section_names[i]);
      all_sections_found = false;
    }
  }
  ASSERT_TRUE(all_sections_found);
}

TEST_F(ElfSectionReaderFixture, TestBogusSection) {
  ASSERT_TRUE(InitializeElfSectionReader());
  ASSERT_FALSE(ReaderHasSection(".bs_loc"));
}

}  // namespace

