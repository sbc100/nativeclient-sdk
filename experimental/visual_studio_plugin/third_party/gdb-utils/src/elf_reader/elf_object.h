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


// This file defines the ElfObject which decodes the elf data and calls
// the appropriate reader callbacks.  The reader interface allows the
// libary user to examine the ELF file without ever needing to know
// machine class (32/64 bit).

#ifndef ELF_READER_ELF_OBJECT_H_
#define ELF_READER_ELF_OBJECT_H_

#include "common/types.h"
#include "elf_reader/elf_structs.h"


namespace elf_reader {

class IElfReader;

// The ElfObject class is used to load and store an ELF based binary.  The
// entire file is read in on a successfull load.  During load, precomputed
// pointers are set to important points in the file.
class ElfObject {
 public:
  enum ClassSize {
    ELFCLASSERR= 0,
    ELFCLASS32 = 1,
    ELFCLASS64 = 2
  };

  enum Encoding {
    ELFDATAERR = 0,
    ELFDATA2LSB = 1,
    ELFDATA2MSB = 2
  };

  enum ObjectType {
    ET_NONE = 0,
    ET_REL  = 1,
    ET_EXEC = 2,
    ET_DYN  = 3
  };

  // These pointers are stored as a union since we sometimes need a byte
  // relative offset, and sometimes need to access either 32b or 64b versions
  // of the structure.  A call to GetClassSize determines if the 32b or 64b
  // version should be used.  These pointers are never provided to the user.
  union ElfHdrPtr {
    uint8_t*  raw_;
    ElfHdrDef* def_;
    ElfHdr32* hdr32_;
    ElfHdr64* hdr64_;
  };

  union ElfShdrPtr {
    uint8_t*  raw_;
    ElfShdr32* shdr32_;
    ElfShdr64* shdr64_;
  };

  union ElfPhdrPtr {
    uint8_t*  raw_;
    ElfPhdr32* phdr32_;
    ElfPhdr64* phdr64_;
  };

 public:
  ElfObject();
  ~ElfObject();

  // Loading a ELF file will cause the entire file to be placed in memory. A
  // call to Unload will free all data associated with the ELF file.  The load
  // function will automatically unload, so it is safe to reuse an ElfObject.
  // The act of loading verifies that the load file containes the expected
  // values and precomputes interesting offsets within the file which are then
  // used by other members.
  bool Load(const char *path);
  void Unload();

  // Retreive information about the loaded file.
  const char *GetPath() const;
  const uint8_t *GetData() const;
  uint64_t GetLength() const;

  // Retrieve ELF header information
  ClassSize GetClassSize() const;
  Encoding GetEncoding() const;
  ObjectType GetObjectType() const;
  uint32_t GetObjectFlags() const;

  // Process the ELF data, using the reader callbacks
  void Parse(IElfReader* reader) const;

 private:
  uint16_t GetProgramHeaderCount() const;
  uint16_t GetSectionHeaderCount() const;
  const char *GetSectionStrings() const;

 private:
  char *path_;
  uint64_t length_;

  ElfHdrPtr  data_;
  ElfObjDef* obj_;
  char *strings_;

  ElfShdrPtr shdr_;
  ElfPhdrPtr phdr_;
};

}  // namespace elf_reader

#endif  // ELF_READER_ELF_OBJECT_H_

