// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwarf_reader/elf_section_reader.h"

namespace dwarf_reader {

ElfSectionReader::ElfSectionReader() : byte_reader_(NULL) {}

ElfSectionReader::~ElfSectionReader() {
  if (byte_reader_)
    delete byte_reader_;
}

const dwarf2reader::SectionMap& ElfSectionReader::sections() const {
  return sections_;
}

void ElfSectionReader::Init(const char *name,
                            void *data,
                            uint64_t length,
                            uint32_t classSize,
                            bool is_little_endian) {
  if (is_little_endian)
    byte_reader_ = new dwarf2reader::ByteReader(
        dwarf2reader::ENDIANNESS_LITTLE);
  else
    byte_reader_ = new dwarf2reader::ByteReader(
        dwarf2reader::ENDIANNESS_BIG);
}

bool ElfSectionReader::SectionHeadersStart(uint32_t count) {
  return true;
}

void ElfSectionReader::AddSectionHeader(const char *name,
                                        void *data,
                                        uint64_t virt,
                                        uint32_t type,
                                        uint32_t flags,
                                        uint64_t length) {
  const char *ptr = reinterpret_cast<const char *>(data);
  sections_[name] = SectionInfo(ptr, length);
  loadAddresses_[name] = virt;
}

dwarf2reader::ByteReader *ElfSectionReader::GetByteReader() const {
  return byte_reader_;
}

SectionInfo ElfSectionReader::GetSectionInfo(const char *name) const {
  dwarf2reader::SectionMap::const_iterator map_iter =
      sections_.find(name);
  if (map_iter != sections_.end())
    return map_iter->second;
  else
    return SectionInfo(NULL, 0);
}

uint64 ElfSectionReader::GetSectionLoadAddress(const char *name) const {
  LoadAddressMap::const_iterator address_iter =
      loadAddresses_.find(name);
  if (address_iter != loadAddresses_.end())
    return address_iter->second;
  else
    return 0;
}

bool ElfSectionReader::IsEmpty() const {
  return (sections_.empty() || loadAddresses_.empty());
}

}  // namespace dwarf_reader
