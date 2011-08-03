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

#include <stack>

#include "common/types.h"
#include "elf_reader/elf_structs.h"
#include "elf_reader/elf_object.h"
#include "elf_reader/elf_reader.h"

#include "dwarf_reader/dwarf_reader.h"
#include "dwarf_reader/dwarf_parse.h"
#include "dwarf_reader/dwarf_vm.h"

#include "common/dwarf/bytereader.h"
#include "common/dwarf/dwarf2reader.h"

using namespace elf_reader;
using namespace dwarf_reader;
using namespace dwarf2reader;

typedef pair<const char *, uint64> SectionInfo;
typedef map<string, uint64> LoadMap;

class ElfSectionReader : public ElfReaderBase {
 public:
  ElfSectionReader() : byteReader_(NULL) {}
  ~ElfSectionReader() { 
    if (byteReader_) 
      delete byteReader_; 
  }

  void Header(const char *name, void *data, uint64_t length, uint32_t classSize, bool lsb) {
    if (lsb)
      byteReader_ = new ByteReader(ENDIANNESS_LITTLE);
    else
      byteReader_ = new ByteReader(ENDIANNESS_BIG);
  }

  bool SectionHeadersStart(uint32_t count) { return true; }
  void SectionHeader(const char *name, void *data, uint64_t virt,
                     uint32_t type, uint32_t flags, uint64_t length) {
    const char *ptr = reinterpret_cast<const char *>(data);
    sections_[name] = SectionInfo(ptr, length);
    loads_[name] = virt;
  }

 public:
  ByteReader *GetByteReader() { return byteReader_; }
  SectionInfo GetSectionInfo(const char *name) { return sections_[name]; }
  uint64 GetSectionLoad(const char *name) { return loads_[name]; }
  SectionMap& GetSectionMap() { return sections_; }
  LoadMap& GetLoadMap() { return loads_; }

 private:
  SectionMap sections_;
  LoadMap loads_;
  ByteReader *byteReader_;
};

struct ParseState {
 public:
  ParseState(IDwarfReader *reader) : reader_(reader), currCU_(NULL) {}

 public:
  IDwarfReader *reader_;
  void *currCU_;
  stack<void*>  ctxStack_;
  stack<uint64> addrStack_;
};

class DwarfInfoParser : public Dwarf2Handler {
 public:
  DwarfInfoParser(ParseState *ps) : ps_(ps) {}


  // Start to process a compilation unit at OFFSET from the beginning of the
  // .debug_info section. Return false if you would like to skip this
  // compilation unit.
  bool StartCompilationUnit(uint64 offset, uint8 address_size,
                            uint8 offset_size, uint64 cu_length,
                            uint8 dwarf_version) {
    void *ctx = ps_->reader_->StartCompilationUnit(offset, address_size,
                                                  offset_size, cu_length,
                                                  dwarf_version);

    ps_->currCU_ = ctx;
    ps_->ctxStack_.push(ctx);
    ps_->addrStack_.push(offset);
    return true;
  }

  bool StartDIE(uint64 offset, enum DwarfTag tag, const AttributeList& attrs) {

    void *ctx = ps_->reader_->StartDIE(ps_->ctxStack_.top(),
                                       ps_->addrStack_.top(),
                                       offset, tag);

    ps_->ctxStack_.push(ctx);
    ps_->addrStack_.push(offset);
    return true;
  }

  void EndDIE(uint64 offset) {

    ps_->reader_->EndDIE(ps_->ctxStack_.top(), ps_->addrStack_.top());
    ps_->ctxStack_.pop();
    ps_->addrStack_.pop();
  }

  void ProcessAttributeUnsigned(uint64 offset, enum DwarfAttribute attr,
                                enum DwarfForm form, uint64 data) {

    ps_->reader_->ProcessAttributeUnsigned(ps_->ctxStack_.top(),
                                           ps_->addrStack_.top(),
                                           offset, attr, form, data);
  }

  void ProcessAttributeSigned(uint64 offset,
                              enum DwarfAttribute attr,
                              enum DwarfForm form,
                              int64 data) {
    ps_->reader_->ProcessAttributeSigned(ps_->ctxStack_.top(),
                                         ps_->addrStack_.top(),
                                         offset, attr, form, data);
  }

  void ProcessAttributeReference(uint64 offset,
                                 enum DwarfAttribute attr,
                                 enum DwarfForm form,
                                 uint64 data) {
    ps_->reader_->ProcessAttributeReference(ps_->ctxStack_.top(),
                                            ps_->addrStack_.top(),
                                            offset, attr, form, data);
  }

  void ProcessAttributeBuffer(uint64 offset,
                              enum DwarfAttribute attr,
                              enum DwarfForm form,
                              const char* data,
                              uint64 len) {
    ps_->reader_->ProcessAttributeBuffer(ps_->ctxStack_.top(),
                                         ps_->addrStack_.top(),
                                         offset, attr, form, data, len);
  }

  void ProcessAttributeString(uint64 offset,
                              enum DwarfAttribute attr,
                              enum DwarfForm form,
                              const string& data) {
    ps_->reader_->ProcessAttributeString(ps_->ctxStack_.top(),
                                         ps_->addrStack_.top(),
                                         offset, attr, form, data.data());
  }

 private:
  ParseState *ps_;
};

class DwarfLineParser : public LineInfoHandler {
 public:
  DwarfLineParser(ParseState *ps) : ps_(ps) {}

  void DefineDir(const string& name, uint32 dir_num) {
    ps_->reader_->DefineDir(ps_->ctxStack_.top(), name.data(), dir_num);
  }

  void DefineFile(const string& name, int32 file_num, uint32 dir_num,
                  uint64 mod_time, uint64 length) {
    ps_->reader_->DefineFile(ps_->ctxStack_.top(), name.data(), file_num,
                             dir_num, mod_time, length);
  }

  void AddLine(uint64 address, uint64 length, uint32 file_num,
               uint32 line_num, uint32 column_num) {

    ps_->reader_->AddLine(ps_->ctxStack_.top(), address, length, file_num,
                          line_num, column_num);
  }


 private:
  ParseState *ps_;
};

class DwarfFrameInfoReader : public CallFrameInfo::Handler {
public:
  DwarfFrameInfoReader(IDwarfReader *reader) : reader_(reader){}

  virtual bool Entry(size_t offset, uint64 address, uint64 length,
    uint8 version, const string &augmentation,
    unsigned return_address){
      return reader_->BeginCfiEntry(
        offset,
        address,
        length,
        version,
        augmentation.c_str(),
        return_address);
  }

  virtual bool UndefinedRule(uint64 address, int reg){
    return reader_->AddCfiRule(
      address,
      reg,
      IDwarfReader::CFIRT_UNDEFINED,
      0,
      0,
      NULL,
      0);
  }

  virtual bool SameValueRule(uint64 address, int reg){
    return reader_->AddCfiRule(
      address,
      reg,
      IDwarfReader::CFIRT_SAMEVALUE,
      0,
      0,
      NULL,
      0);
  }

  virtual bool OffsetRule(uint64 address, int reg,
    int base_register, long offset){
      return reader_->AddCfiRule(
        address,
        reg,
        IDwarfReader::CFIRT_OFFSET,
        base_register,
        offset,
        NULL,
        0);
  }

  virtual bool ValOffsetRule(uint64 address, int reg,
    int base_register, long offset){
      return reader_->AddCfiRule(
        address,
        reg,
        IDwarfReader::CFIRT_VALOFFSET,
        base_register,
        offset,
        NULL,
        0);
  }

  virtual bool RegisterRule(uint64 address, int reg, int base_register){
    return reader_->AddCfiRule(
      address,
      reg,
      IDwarfReader::CFIRT_REGISTER,
      base_register,
      0,
      NULL,
      0);
  }

  virtual bool ExpressionRule(uint64 address, int reg,
    const string &expression){
      return reader_->AddCfiRule(
        address,
        reg,
        IDwarfReader::CFIRT_EXPRESSION,
        0,
        0,
        expression.c_str(),
        expression.length());
  }

  virtual bool ValExpressionRule(uint64 address, int reg,
    const string &expression){
      return reader_->AddCfiRule(
        address,
        reg,
        IDwarfReader::CFIRT_VALEXPRESSION,
        0,
        0,
        expression.c_str(),
        expression.length());
  }

  virtual bool End(){
    return reader_->EndCfiEntry();
  }

private:
  IDwarfReader* reader_;
};



namespace dwarf_reader {

void DwarfParseElf(ElfObject *elf, IDwarfReader *reader) {
  ElfSectionReader *elfInfo = new ElfSectionReader();

  ParseState parseState(reader);

  if (NULL == elf)
    return;

  if (NULL == reader)
    return;

  if (NULL == elfInfo)
    return;

  elf->Parse(elfInfo);
  SectionInfo debug_info_section = elfInfo->GetSectionInfo(".debug_info");
  SectionInfo debug_line_section = elfInfo->GetSectionInfo(".debug_line");
  SectionInfo debug_loc_section = elfInfo->GetSectionInfo(".debug_loc");
  SectionInfo debug_frame_section = elfInfo->GetSectionInfo(".eh_frame");
  SectionInfo text_section = elfInfo->GetSectionInfo(".text");

  elfInfo->GetByteReader()->SetTextBase((uint64)text_section.first);
  elfInfo->GetByteReader()->SetCFIDataBase(elfInfo->GetSectionLoad(".eh_frame"), debug_frame_section.first);

  DwarfInfoParser info_handler(&parseState);
  DwarfLineParser line_handler(&parseState);

  uint64 debug_info_length = debug_info_section.second;
  uint64 debug_line_length = debug_line_section.second;
  const char *debug_line_ptr = debug_line_section.first;
  for (uint64 offset = 0; offset < debug_info_length;) {
    dwarf2reader::CompilationUnit cureader(elfInfo->GetSectionMap(),
                                           offset,
                                           elfInfo->GetByteReader(),
                                           &info_handler);

    // Process the entire compilation unit; get the offset of the next.
    offset += cureader.Start();

    // Process the matching line information; get the offset of the next.
    dwarf2reader::LineInfo lineInfo(debug_line_ptr, debug_line_length,
                                    elfInfo->GetByteReader(), &line_handler);

    debug_line_ptr += lineInfo.Start();

    // Pop the end of the compilation unit manually
    reader->EndCompilationUnit(parseState.ctxStack_.top(), parseState.addrStack_.top());
    parseState.ctxStack_.pop();
    parseState.addrStack_.pop();
  }

  //
  // Read the call frame information
  //
  dwarf2reader::CallFrameInfo::Reporter reporter(elf->GetPath());
  DwarfFrameInfoReader handler(reader);
  dwarf2reader::CallFrameInfo cfiReader(
    debug_frame_section.first,
    debug_frame_section.second,
    elfInfo->GetByteReader(),
    &handler,
    &reporter,
    true);
  cfiReader.Start();

  //
  // Read the location list
  // TODO(ilewis): this should probably move to a different function.
  //
  const char* debug_loc_ptr = debug_loc_section.first;
  const char* current = debug_loc_ptr;
  uint64 debug_loc_length = debug_loc_section.second;
  const char* debug_loc_end = debug_loc_ptr + debug_loc_length;
  ByteReader* byte_reader = elfInfo->GetByteReader();
  bool is_first = true;
  while (current < debug_loc_end) {
    //
    // Layout of the debug_loc block is:
    //
    //  LowPc       - address
    //  HighPc      - address
    //  DataLength  - ushort (optional)
    //  Data        - byte[] (optional)
    //
    uint64 offset = current - debug_loc_ptr;
    
    uint64 lowPc = byte_reader->ReadAddress(current);
    current += byte_reader->AddressSize();

    uint64 highPc = byte_reader->ReadAddress(current);
    current += byte_reader->AddressSize();

    size_t dataSize = 0;
    const void* data = 0;

    if (lowPc == 0 && highPc == 0) {
      // if lowPc and highPc are both zero, that signals end of list.
      is_first = true;
      continue;
    } else if (lowPc == (uint64) -1) {
      // the location is an absolute address; its value is in highPc.
      dataSize = 4;
      data = &highPc;
    } else {
      dataSize = byte_reader->ReadTwoBytes(current);
      current += 2;

      data = (void*)current;
      current += dataSize;
    }

    reader->AddLocListEntry(offset, is_first, lowPc, highPc, data, dataSize);

    is_first = false;
  }

  delete elfInfo;
}

uint32_t DwarfRun32(IDwarfVM *vm, ByteReader *reader, const char *program, int length);
uint64_t DwarfRun64(IDwarfVM *vm, ByteReader *reader, const char *program, int length);

uint64_t DwarfParseVM(IDwarfVM* vm, uint8_t *data, uint32_t length) {
  ByteReader *byteReader;
  if (vm->IsLSB())
      byteReader = new ByteReader(ENDIANNESS_LITTLE);
    else
      byteReader = new ByteReader(ENDIANNESS_BIG);

  switch(vm->BitWidth()) {
      case 32: 
        byteReader->SetAddressSize(4);
        return DwarfRun32(vm, byteReader, (const char *) data, length);
        break;

      case 64:
        byteReader->SetAddressSize(8);
        return DwarfRun32(vm, byteReader, (const char *) data, length);
        break;
  }

  return 0;
}
  

}  // namespace dwarf_reader

