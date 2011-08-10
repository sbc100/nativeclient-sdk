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

void ElfSectionReader::Init(const char *name,
                            void *data,
                            uint64_t length,
                            uint32_t classSize,
                            bool is_linux_standard_base) {
  if (is_linux_standard_base)
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

dwarf2reader::ByteReader *ElfSectionReader::GetByteReader() {
  return byte_reader_;
}

SectionInfo ElfSectionReader::GetSectionInfo(const char *name) {
  return sections_[name];
}

uint64 ElfSectionReader::GetSectionLoadAddress(const char *name) {
  return loadAddresses_[name];
}

dwarf2reader::SectionMap& ElfSectionReader::GetSectionMap() {
  return sections_;
}

}  // namespace dwarf_reader
