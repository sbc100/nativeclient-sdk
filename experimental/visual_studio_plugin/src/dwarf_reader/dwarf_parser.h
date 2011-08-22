// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_DWARF_PARSER_H_
#define DWARF_READER_DWARF_PARSER_H_

namespace elf_reader {
  class ElfObject;
}  // namespace elf_reader

namespace dwarf_reader {
class IDwarfReader;
class ElfSectionReader;

class DwarfParser {
 public:
  DwarfParser();
  virtual ~DwarfParser();

  /// Initializes the instance of the class with a file path and by populating
  /// an |ElfSectionReader|.
  /// @param[in] elf_object The |ElfObject|, populated with a file location.
  /// @param[in] file_path The file location that |elf_object| was populated
  /// with.
  /// @return false iff there is a sign that something may have gone wrong or
  /// no memory was available.
  bool Init(elf_reader::ElfObject *elf_object);

  /// Populates an instance of |IDwarfReader| with debug information.
  /// @param[out] dwarf_reader the object to be populated.
  void PopulateReader(IDwarfReader *dwarf_reader) const;

 private:
  /// Populates an instance of IDwarfReader with the call frame information
  /// from the |elf_object|.
  /// @param[out] dwarf_reader the object to be populated.
  void PopulateCallFrameInfo(IDwarfReader *dwarf_reader) const;

  /// Populates an instance of IDwarfReader with the compilation unit
  /// information from the |elf_object|.
  /// @param[out] dwarf_reader the object to be populated.
  void PopulateCompilationUnits(IDwarfReader *dwarf_reader) const;

  /// Populates an instnace of IDwarfReader with the Location List information
  /// from the |elf_object|.
  /// @param[out] dwarf_reader the object to be populated.
  void PopulateLocationLists(IDwarfReader *dwarf_reader) const;

  /// Populates an instance of IDwarfReader with the Range List information
  /// from the |elf_object|.
  /// @param[out] dwarf_reader the object to be populated.
  void PopulateRangeLists(IDwarfReader *dwarf_reader) const;

  /// The location of the binary that this parser is parsing.
  const char *file_path_;
  /// The |ElfSectionReader| that is used to find the correct parts of the
  /// file.
  ElfSectionReader *elf_section_reader_;

  bool is_initialized_;
};
}  // namespace dwarf_reader

#endif  // DWARF_READER_DWARF_PARSER_H_
