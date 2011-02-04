// -*- mode: c++ -*-

// Copyright (c) 2010 Google Inc. All Rights Reserved.
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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include <algorithm>
#include <map>
#include <stack>
#include <iostream>
#include <sstream>

#include "common\dwarf\dwarf2enums.h"

#include "elf_reader\elf_object.h"
#include "dwarf_reader\dwarf_parse.h"
#include "dwarf_reader\dwarf_reader.h"
#include "dwarf_reader\dwarf_strings.h"
#include "dwarf_reader\dwarf_vm.h"

using namespace std;
using namespace dwarf2reader;
using namespace dwarf_reader;
using namespace elf_reader;

void StreamTabs(uint32_t depth) {
  cout.width(0);

  for (int64 a = 0; a < depth; a++)
    cout << "  ";
}

void StreamStartTag(const char *name) {
  cout.width(0);
  cout << "<" << name << ">";
}

void StreamEndTag(const char *name) {
  cout.width(0);
  cout << "</" << name << ">" << endl;
}

template<typename T>
void StreamValue(const T& val) {
  cout << val;
}

void StreamValue(const uint64& val) {
  cout << hex << val << dec;
}

template<typename T>
void StreamTag(int tabs, const char* name, const T& val) {
  StreamTabs(tabs);
  StreamStartTag(name);
  StreamValue(val);
  StreamEndTag(name);
}

template<typename T>
void StreamTag(const char* name, const T& val, int tabs = 0) {
  StreamTag(0, name, val);
}


class DwarfInfo : public IDwarfReader {
public:
  void *StartCompilationUnit(uint64 offset, uint8 address_size,
                                    uint8 offset_size, uint64 cu_length,
                                    uint8 dwarf_version) {

    cout.fill('0');
    cout.setf(std::ios_base::showbase);

    StreamStartTag("compile_block");
    cout << endl;
    return (void *) 1;
  }

  void EndCompilationUnit(void *ctx, uint64 offset) {
    StreamEndTag("compile_block");
  }
  
  void *StartDIE(void *ctx, uint64 parent, uint64 offset, enum DwarfTag tag) {
    const char *tagName = DwarfTagName(tag);
   
    StreamTabs((int) ctx);
    StreamStartTag(tagName);
    cout << endl;

    tags_.push(tagName);

    StreamTabs((int) ctx + 1);
    StreamStartTag("ID");
    cout.width(1);
    cout.setf(ios_base::hex , ios_base::basefield);
    cout << offset;
    StreamEndTag("ID");
    return (void *) (((int) ctx) + 1);
  }

  void EndDIE(void *ctx, uint64 offset) {
    const char *name = tags_.top();
    tags_.pop();

    StreamTabs((int) ctx - 1);
    StreamStartTag(name);
    cout << endl;
  }

  void ProcessAttributeUnsigned(void *ctx,
                                        uint64 offset,
                                        uint64 parent, 
                                        enum DwarfAttribute attr,
                                        enum DwarfForm form,
                                        uint64 data) {
    const char *attrName = DwarfAttributeName(attr);

    switch(form) {
       case DW_FORM_data8:
        cout.width(16);
        cout.setf(ios_base::hex , ios_base::basefield);
        break;

       case DW_FORM_data4:
       case DW_FORM_data2:
        cout.width(8);
        cout.setf(ios_base::hex , ios_base::basefield);
        break;
    
      case DW_FORM_addr:
        cout.width(1);
        cout.setf(ios_base::hex , ios_base::basefield);
        break;

      case DW_FORM_data1:
      case DW_FORM_flag:
        cout.width(1);
        cout.setf(ios_base::dec , ios_base::basefield);
        break;

      default:
        break;
    }

    StreamTabs((int) ctx);
    StreamStartTag(attrName);
    cout << data;
    StreamEndTag(attrName);
  }

  void ProcessAttributeSigned(void *ctx,
                                      uint64 offset,
                                      uint64 parent, 
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      int64 data) {
    const char *attrName = DwarfAttributeName(attr);
    switch(form) {
       case DW_FORM_data4:
       case DW_FORM_data2:
        cout.width(8);
        cout.setf(ios_base::hex , ios_base::basefield);
         break;
    
      case DW_FORM_data1:
      case DW_FORM_sdata:
        cout.width(1);
        cout.setf(ios_base::dec , ios_base::basefield);
         break;

      default:
        break;
    }

    StreamTabs((int) ctx);
    StreamStartTag(attrName);
    cout << data;
    StreamEndTag(attrName);
  }

  void ProcessAttributeReference(void *ctx,
                                         uint64 offset,
                                         uint64 parent, 
                                         enum DwarfAttribute attr,
                                         enum DwarfForm form,
                                         uint64 data) {
    const char *attrName = DwarfAttributeName(attr);
    switch(form) {
      case DW_FORM_ref_addr:
      case DW_FORM_ref1:
      case DW_FORM_ref2:
      case DW_FORM_ref4:
      case DW_FORM_ref8:
        cout.width(1);
        cout.setf(ios_base::hex , ios_base::basefield);
        break;

      default:
        break;
    }

    StreamTabs((int) ctx);
    StreamStartTag(attrName);
    cout << data;
    StreamEndTag(attrName);
  }

  void ProcessAttributeBuffer(void *ctx,
                                      uint64 offset,
                                      uint64 parent, 
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const char* data,
                                      uint64 len) {
    const char *attrName = DwarfAttributeName(attr);

    StreamTabs((int) ctx);
    StreamStartTag(attrName);

    cout.width(0);
    cout.setf(ios_base::hex , ios_base::basefield);

    if (form != DW_FORM_block1) {
      printf("ATTR=%s FORM=%s\n", DwarfAttributeName(attr), DwarfFormName(form));
      cout.unsetf(std::ios_base::showbase);
      cout << "0x";
      for (uint64 a = 0; a < len; a++) {    
        uint32 l = ((unsigned char) data[a]) >> 4;
        uint32 r = ((unsigned char) data[a]) & 0xF;

        cout << l;
        cout << r;
      }
    } 
    else {  
      DwarfStaticVM64 vm;
//    vm.Push(0);
//    vm.Run(data, len);
//    int64 addr = *(vm.StackTop());
      
      int64 addr = DwarfParseVM(&vm, (uint8_t *) data, len);
      cout.setf(std::ios_base::showbase);
      if (addr < 0)
        cout.setf(ios_base::dec , ios_base::basefield);

      cout << addr;
    }
    cout.setf(std::ios_base::showbase);
    StreamEndTag(attrName);
  }

  void ProcessAttributeString(void *ctx,
                                      uint64 offset,
                                      uint64 parent, 
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const char* data) {
    const char *attrName = DwarfAttributeName(attr);
    switch(form) {
      case DW_FORM_strp:
      case DW_FORM_string:
        break;

      default:
        break;
    }

    StreamTabs((int) ctx);
    StreamStartTag(attrName);
    cout << data;  
    StreamEndTag(attrName);
  }
  
  void DefineDir(void *ctx, const char *name, uint32 dir_num) {
    StreamTabs((int) ctx);
    StreamStartTag("dir");
    cout << endl;

    StreamTabs((int) ctx + 1);
    StreamStartTag("name");
    cout << name;  
    StreamEndTag("name");

    StreamTabs((int) ctx + 1);
    StreamStartTag("number");
    cout << dir_num;  
    StreamEndTag("number");

    StreamTabs((int) ctx);
    StreamEndTag("dir");
  }

  void DefineFile(void *ctx, const char *name, int32 file_num,
                  uint32 dir_num, uint64 mod_time, uint64 length) {

    StreamTabs((int) ctx);
    StreamStartTag("file");
    cout << endl;

    StreamTabs((int) ctx + 1);
    StreamStartTag("name");
    cout << name;  
    StreamEndTag("name");

    StreamTabs((int) ctx + 1);
    StreamStartTag("number");
    cout << file_num;  
    StreamEndTag("number");

    StreamTabs((int) ctx + 1);
    StreamStartTag("dir");
    cout << dir_num;  
    StreamEndTag("dir");

    StreamTabs((int) ctx);
    StreamEndTag("file");
  }

  void AddLine(void *ctx, uint64 address, uint64 length, uint32 file_num, 
               uint32 line_num, uint32 column_num) {

    StreamTabs((int) ctx);
    StreamStartTag("line");
    cout << endl;

    StreamTabs((int) ctx + 1);
    StreamStartTag("address");
    cout.setf(ios_base::hex , ios_base::basefield);
    cout << address;  
    StreamEndTag("address");

    StreamTabs((int) ctx + 1);
    StreamStartTag("length");
    cout.setf(ios_base::dec , ios_base::basefield);
    cout << length;  
    StreamEndTag("length");

    StreamTabs((int) ctx + 1);
    StreamStartTag("file");
    cout << file_num;  
    StreamEndTag("file");

    StreamTabs((int) ctx + 1);
    StreamStartTag("line_number");
    cout << line_num;  
    StreamEndTag("line_number");

    StreamTabs((int) ctx);
    StreamEndTag("line");
  }

  virtual void AddLocListEntry(
    uint64 offset,
    bool is_first_entry,
    uint64 lowPc,
    uint64 highPc,
    const void* data,
    size_t dataSize) {
      // do nothing
  }

  virtual bool BeginCfiEntry (
    size_t offset, 
    uint64 address, 
    uint64 length,
    uint8 version, 
    const char* augmentation,
    unsigned return_address){
      StreamStartTag("cfi-entry");
      cout << endl;
      StreamTag(1, "offset", offset);
      StreamTag(1, "address", address);
      StreamTag(1, "version", (uint32)version);
      StreamTag(1, "augmentation", augmentation);
      StreamTag(1, "return-address", return_address);
      StreamTabs(1);
      StreamStartTag("rules");
      cout << endl;
      return true;
  }

  virtual bool AddCfiRule(
    uint64 address,
    int reg,
    CFI_RuleType ruleType,
    int base_register,
    int32 offset,
    const void* expression,
    uint32 expressionLength) {
      StreamTabs(2);
      StreamStartTag("cfi-rule");
      cout << endl;

      StreamTag(3, "address", address);
      StreamTag(3, "register", reg);
      StreamTag(3, "rule-type", ruleType);
      StreamTag(3, "base-register", base_register);
      StreamTag(3, "offset", offset);
      StreamTag(3, "expression", expression);

      StreamTabs(2);
      StreamEndTag("cfi-rule");
      return true;
  }

  virtual bool EndCfiEntry() {
    StreamTabs(1);
    StreamEndTag("rules");
    StreamEndTag("cfi-entry");
    return true;
  }
public:
  stack<const char *> tags_;
};


 
int main(int argc, const char *argv[]) {
  const char *file  = "..\\..\\..\\..\\src\\loop\\loop.nexe";
  ElfObject elf;
  DwarfInfo dwarf;

  if (argc == 2)
    file = argv[1];
  else {
    printf("Expecting: dwarf_xml <file.nexe>\n");
    return -1;
  }

  if (0 == file)
    return -1;

  if (elf.Load(file)) 
    DwarfParseElf(&elf, &dwarf);
  else 
    printf("Could not load '%s'\n", file);

  return 0;
}
