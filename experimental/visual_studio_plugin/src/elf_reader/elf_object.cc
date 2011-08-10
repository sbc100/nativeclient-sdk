// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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

  if (NULL == path) return false;

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
    if (length_ < sizeof(ElfHdrDef)) {
      CleanUp(fp);
      return false;
    }

    // make sure we allocated enough
    void *data = malloc(static_cast<size_t>(length_));
    if (NULL == data) {
      CleanUp(fp);
      return false;
    }
    data_.raw_ = reinterpret_cast<uint8_t *>(data);

    // Make sure we load the whole file
    size_t loaded = fread(data_.raw_, 1, length_, fp);
    if (loaded != length_) {
      CleanUp(fp);
      return false;
    }

    // Verify we have the expected ELF indentifier at the head of the file
    if (memcmp(data_.def_->e_ident, s_ident, sizeof(s_ident))) {
      CleanUp(fp);
      return false;
    }

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
        if (data_.hdr32_->e_obj.e_shentsize != sizeof(ElfShdr32) ||
            data_.hdr32_->e_obj.e_phentsize != sizeof(ElfPhdr32)) {
          CleanUp(fp);
          return false;
        }
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
        if (data_.hdr64_->e_obj.e_shentsize != sizeof(ElfShdr64) ||
            data_.hdr64_->e_obj.e_phentsize != sizeof(ElfPhdr64)) {
            CleanUp(fp);
            return false;
        }
        // If everything checks out, we can precompute strings offset
        uint64_t offset = shdr_.shdr64_[strndx].sh_offset;
        strings_ = reinterpret_cast<char *>(&data_.raw_[ offset ]);
        break;
      }

    }

    fclose(fp);
    return true;
  }

  return false;
}

void ElfObject::CleanUp(FILE *fp) {
  Unload();
  fclose(fp);
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
  reader->Init(path_, data_.raw_, length_, GetClassSize(), lsb);

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
      reader->AddSectionHeader(path, start, virt, type, flags, length);
    }
    reader->SectionHeadersEnd();
  }
}

}  // namespace elf_reader

