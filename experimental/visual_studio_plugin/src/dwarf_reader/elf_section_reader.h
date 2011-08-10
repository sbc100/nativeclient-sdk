// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_ELF_SECTION_READER_H_
#define DWARF_READER_ELF_SECTION_READER_H_

#include <map>
#include <string>

#include "common/dwarf/bytereader.h"
#include "common/dwarf/dwarf2reader.h"

#include "elf_reader/elf_reader.h"

using elf_reader::IElfReader;

namespace dwarf_reader {
/// Maps load addresses to a section names.  This refers to the address of the
/// section in memory.
typedef std::map<std::string, uint64> LoadAddressMap;
/// The name and address of a file section.  This refers to the address of the
/// section in the file text.
typedef std::pair<const char *, uint64> SectionInfo;

/// The purpose of this class is to parse a raw ELF binary file and then to
/// provide an interface for extracting section and program header information
/// from it.
/// For documentation on the function, see elf_reader/elf_reader.h
class ElfSectionReader : public IElfReader {
 public:
  ElfSectionReader();

  ~ElfSectionReader();

  virtual void Init(const char *name,
                    void *data,
                    uint64_t length,
                    uint32_t classSize,
                    bool is_linux_standard_base);

  virtual bool SectionHeadersStart(uint32_t count);
  virtual void SectionHeadersEnd() { }
  virtual void AddSectionHeader(const char *name,
                                void *data,
                                uint64_t virt,
                                uint32_t type,
                                uint32_t flags,
                                uint64_t length);

  /// Not thread safe.
  /// @return A pointer to a ByteReader.
  dwarf2reader::ByteReader *GetByteReader();

  /// Retrieves a descriptor for a section in the ELF binary file.
  /// @param name The name of the section for which information is required.
  /// @return SectionInfo for the requested section.
  SectionInfo GetSectionInfo(const char *name);

  /// @param name The name of the section for which load info is required.
  /// @return The load address of the section.
  uint64 GetSectionLoadAddress(const char *name);

  /// @return The SectionMap for the file.
  dwarf2reader::SectionMap& GetSectionMap();

 private:
  dwarf2reader::SectionMap sections_;
  LoadAddressMap loadAddresses_;
  dwarf2reader::ByteReader *byte_reader_;
};

}  // namespace dwarf_reader

#endif  // DWARF_READER_ELF_SECTION_READER_H_
