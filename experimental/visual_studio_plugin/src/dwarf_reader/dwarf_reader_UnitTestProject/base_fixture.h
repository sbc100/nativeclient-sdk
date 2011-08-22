// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_DWARF_READER_UNITTESTPROJECT_BASE_FIXTURE_H_
#define DWARF_READER_DWARF_READER_UNITTESTPROJECT_BASE_FIXTURE_H_

#include <errno.h>
#include <string>

#include "elf_reader/elf_object.h"
#include "gtest/gtest.h"

namespace dwarf_reader_tests {

/// The tests share some basic setup that can be pushed up into this base
/// class.
class BaseFixture : public ::testing::Test {
 protected:
  virtual ~BaseFixture() {}

  /// Attempts to retrieve the root directory of the VSX plugin source
  /// directory from the environment.
  /// @return False if it can't.
  bool InitializeNaClVSXRoot() {
    bool success = false;
    char *dupenv_buffer;
    std::size_t buffer_size;
    errno_t status = _dupenv_s(&dupenv_buffer, &buffer_size, "NACL_VSX_ROOT");
    if (ENOMEM != status) {
      nacl_vsx_root_ = dupenv_buffer;
      success = true;
    }
    free(dupenv_buffer);
    return success;
  }

  /// Constructs an elf object.
  /// @return false if it can't.
  bool InitializeElfObject() {
    bool success = false;
    if (InitializeNaClVSXRoot()) {
      loop_nexe_path_ = nacl_vsx_root_ + "\\src\\loop\\loop.nexe";
      success = elf_object_.Load(loop_nexe_path_.c_str());
    }
    return success;
  }

  elf_reader::ElfObject elf_object_;
  std::string nacl_vsx_root_;
  std::string loop_nexe_path_;
};

}  // namespace dwarf_reader_tests

#endif  // DWARF_READER_DWARF_READER_UNITTESTPROJECT_BASE_FIXTURE_H_
