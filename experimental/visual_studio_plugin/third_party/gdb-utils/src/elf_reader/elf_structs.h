// Copyright 2010 Google, Inc.  All Rights reserved
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


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

