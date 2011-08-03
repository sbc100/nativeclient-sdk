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
//     * Neither the path of Google Inc. nor the paths of its
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common/types.h"
#include "elf_reader/elf_structs.h"
#include "elf_reader/elf_object.h"
#include "elf_reader/elf_reader.h"

namespace elf_reader {

static uint8_t s_ident[4] = { 0x7F, 'E', 'L', 'F' };

ElfObject::ElfObject()
  : path_(NULL),
    length_(0),
    obj_(NULL),
    strings_(NULL) {
  data_.raw_ = 0;
  shdr_.raw_ = 0;
  phdr_.raw_ = 0;
}


ElfObject::~ElfObject() {
  Unload();
}

void ElfObject::Unload() {
  if (data_.raw_)
    free(data_.raw_);

  if (path_)
    free(path_);

  path_ = NULL;
  length_ = 0;

  data_.raw_ = NULL;
  shdr_.raw_ = NULL;
  phdr_.raw_ = NULL;
  strings_ = NULL;
}

bool ElfObject::Load(const char *path) {
  Unload();

  if (NULL == path)
    return false;

  FILE *fp = fopen(path, "rb");
  if (fp) {
    // Copy the path
    path_ = reinterpret_cast<char *>(malloc(strlen(path) + 1));
    strcpy(path_, path);

    // Determine the length
    fseek(fp, 0, SEEK_END);
    length_ = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /// Verify this is large enough to process the headers
    if (length_ < sizeof(ElfHdrDef))
      goto failed;

    // make sure we allocated enough
    void *data = malloc(static_cast<size_t>(length_));
    if (NULL == data)
      goto failed;
    data_.raw_ = reinterpret_cast<uint8_t *>(data);

    // Make sure we load the whole file
    size_t loaded = fread(data_.raw_, 1, length_, fp);
    if (loaded != length_)
      goto failed;

    // Verify we have the expected ELF indentifier at the head of the file
    if (memcmp(data_.def_->e_ident, s_ident, sizeof(s_ident)))
      goto failed;

    // Set precomputed pointers and verify structure sizes
    switch (GetClassSize()) {
      case ELFCLASS32: {
        // Get the index to the section containing section names
        uint32_t strndx = data_.hdr32_->e_obj.e_shstrndx;

        // Set precomputed header offsets
        shdr_.raw_ = &data_.raw_[data_.hdr32_->e_shoff];
        phdr_.raw_ = &data_.raw_[data_.hdr32_->e_phoff];
        obj_ = &data_.hdr32_->e_obj;

        // Verify headers are the expected sizes for this Class (machine width)
        if (data_.hdr32_->e_obj.e_shentsize != sizeof(ElfShdr32))
          goto failed;
        if (data_.hdr32_->e_obj.e_phentsize != sizeof(ElfPhdr32))
          goto failed;

        // If everything checks out, we can precompute strings offset
        uint8_t *string_offs = &data_.raw_[ shdr_.shdr32_[strndx].sh_offset ];
        strings_ = reinterpret_cast<char *>(string_offs);
        break;
      }


      case ELFCLASS64: {
        // Get the index to the section containing section names
        uint32_t strndx = data_.hdr64_->e_obj.e_shstrndx;

        // Set precomputed offsets
        shdr_.raw_ = &data_.raw_[data_.hdr64_->e_shoff];
        phdr_.raw_ = &data_.raw_[data_.hdr64_->e_phoff];
        obj_ = &data_.hdr64_->e_obj;

        // Verify headers are the expected sizes for this Class (machine width)
        if (data_.hdr64_->e_obj.e_shentsize != sizeof(ElfShdr64))
          goto failed;
        if (data_.hdr64_->e_obj.e_phentsize != sizeof(ElfPhdr64))
          goto failed;

        // If everything checks out, we can precompute strings offset
        uint64_t offset = shdr_.shdr64_[strndx].sh_offset;
        strings_ = reinterpret_cast<char *>(&data_.raw_[ offset ]);
        break;
      }

      // Unknown class
      default:
        goto failed;
    }

    fclose(fp);
    return true;

 failed:
    Unload();
    fclose(fp);
  }

  return false;
}

ElfObject::ClassSize ElfObject::GetClassSize() const {
  if (data_.def_)
    return static_cast<ElfObject::ClassSize>(data_.def_->e_class);

  return ELFCLASSERR;
}

ElfObject::Encoding ElfObject::GetEncoding() const {
  if (data_.def_)
    return static_cast<ElfObject::Encoding>(data_.def_->e_encoding);

  return ELFDATAERR;
}

ElfObject::ObjectType ElfObject::GetObjectType() const {
  if (data_.def_)
    return static_cast<ElfObject::ObjectType>(data_.def_->e_type);

  return ET_NONE;
}


const char * ElfObject::GetPath() const {
  return path_;
}

const uint8_t *ElfObject::GetData() const {
  return data_.raw_;
}

uint16_t ElfObject::GetProgramHeaderCount() const {
  if (obj_) {
    return obj_->e_phnum;
  }

  return 0;
}

uint16_t ElfObject::GetSectionHeaderCount() const {
  if (obj_) {
    return obj_->e_shnum;
  }

  return 0;
}

void ElfObject::Parse(IElfReader* reader) const {
  if (NULL == reader)
    return;

  bool lsb = GetEncoding() == ELFDATA2LSB;
  reader->Header(path_, data_.raw_, length_, GetClassSize(), lsb);

  if (reader->SectionHeadersStart(GetSectionHeaderCount())) {
    // NOTE: Section index 0, 0xFF00-0xFFFF are special, so we
    //      skip '0' since it is invalid and zero filled.
    uint32_t a;
    for (a = 1; a < GetSectionHeaderCount(); a++) {
      const char *path;
      uint32_t type;
      uint32_t flags;
      void* start;
      uint64_t length;
      uint64_t virt;

      switch (GetClassSize()) {
        case ELFCLASS32: {
          ElfShdr32 *sec = &shdr_.shdr32_[a];
          path = &strings_[sec->sh_name];
          flags = sec->sh_flags;
          length= sec->sh_size;
          start = &data_.raw_[sec->sh_offset];
          type = sec->sh_type;
          virt = sec->sh_addr;
          break;
        }

        case ELFCLASS64: {
          ElfShdr64 *sec = &shdr_.shdr64_[a];
          path = &strings_[sec->sh_name];
          flags = static_cast<uint32_t>(sec->sh_flags);
          length= sec->sh_size;
          start = &data_.raw_[sec->sh_offset];
          type = sec->sh_type;
          virt = sec->sh_addr;
          break;
        }

        // This is actually unreachible since we verified the class on load.
        default:
          continue;
      }
      reader->SectionHeader(path, start, virt, type, flags, length);
    }
    reader->SectionHeadersEnd();
  }

  if (reader->ProgramHeadersStart(GetProgramHeaderCount())) {
    uint32_t a;
    for (a = 0; a < GetProgramHeaderCount(); a++) {
      uint32_t type;
      uint32_t flags;
      uint64_t offset;
      uint64_t vaddr;
      uint64_t fsize;
      uint64_t msize;

      switch (GetClassSize()) {
        case ELFCLASS32: {
          ElfPhdr32 *prg = &phdr_.phdr32_[a];
          type = prg->p_type;
          flags = prg->p_flags;
          offset = prg->p_offset;
          vaddr = prg->p_vaddr;
          fsize = prg->p_filesz;
          msize = prg->p_memsz;
          break;
        }

        case ELFCLASS64: {
          ElfPhdr64 *prg = &phdr_.phdr64_[a];
          type = prg->p_type;
          flags = prg->p_flags;
          offset = prg->p_offset;
          vaddr = prg->p_vaddr;
          fsize = prg->p_filesz;
          msize = prg->p_memsz;
          break;
        }

        // This is actually unreachible since we verified the class on load.
        default:
          continue;
      }
      reader->ProgramHeader(type, flags, offset, vaddr, fsize, msize);
    }
    reader->ProgramHeadersEnd();
  }
}

}  // namespace elf_reader

