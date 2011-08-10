// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// This file contains the structures define by ELF.


#ifndef ELF_READER_ELF_STRUCTS_H_
#define ELF_READER_ELF_STRUCTS_H_

#include "common/types.h"

namespace elf_reader {

// For convienance the common parts between 32 and 64 bit versions
// of the ELF header have been broken out into seperate structures.
struct ElfHdrDef {
  uint8_t  e_ident[4];
  uint8_t  e_class;
  uint8_t  e_encoding;
  uint8_t  e_fversion;
  uint8_t  e_abi;
  uint8_t  e_pad[8];
  uint16_t  e_type;
  uint16_t  e_machine;
  uint32_t  e_version;
};

struct ElfObjDef {
  uint32_t  e_flags;
  uint16_t  e_ehsize;
  uint16_t  e_phentsize;
  uint16_t  e_phnum;
  uint16_t  e_shentsize;
  uint16_t  e_shnum;
  uint16_t  e_shstrndx;
};

struct ElfHdr32 {
  ElfHdrDef  e_def;
  uint32_t  e_entry;
  uint32_t  e_phoff;
  uint32_t  e_shoff;
  ElfObjDef  e_obj;
};

struct ElfHdr64 {
  ElfHdrDef  e_def;
  uint64_t  e_entry;
  uint64_t  e_phoff;
  uint64_t  e_shoff;
  ElfObjDef  e_obj;
};

struct ElfShdr32 {
  uint32_t  sh_name;
  uint32_t  sh_type;
  uint32_t  sh_flags;
  uint32_t  sh_addr;
  uint32_t  sh_offset;
  uint32_t  sh_size;
  uint32_t  sh_link;
  uint32_t  sh_info;
  uint32_t  sh_addralign;
  uint32_t  sh_entsize;
};

struct ElfShdr64 {
  uint32_t  sh_name;
  uint32_t  sh_type;
  uint64_t  sh_flags;
  uint64_t  sh_addr;
  uint64_t  sh_offset;
  uint64_t  sh_size;
  uint32_t  sh_link;
  uint32_t  sh_info;
  uint64_t  sh_addralign;
  uint64_t  sh_entsize;
};

struct ElfPhdr32 {
  uint32_t  p_type;
  uint32_t  p_offset;
  uint32_t  p_vaddr;
  uint32_t  p_paddr;
  uint32_t  p_filesz;
  uint32_t  p_memsz;
  uint32_t  p_flags;
  uint32_t  p_align;
};

struct ElfPhdr64 {
  uint32_t  p_type;
  uint32_t  p_flags;
  uint64_t  p_offset;
  uint64_t  p_vaddr;
  uint64_t  p_paddr;
  uint64_t  p_filesz;
  uint64_t  p_memsz;
  uint64_t  p_align;
};

}  // namespace elf_reader

#endif  // ELF_READER_ELF_STRUCTS_H_

