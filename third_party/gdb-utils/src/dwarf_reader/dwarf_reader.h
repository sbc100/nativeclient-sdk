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


// This file contains the interface for processing DWARF Data

#ifndef DWARF_READER_DWARF_READER_H_
#define DWARF_READER_DWARF_READER_H_

#include "common/types.h"
#include "common/dwarf/dwarf2enums.h"

namespace dwarf_reader {

using dwarf2reader::DwarfAttribute;
using dwarf2reader::DwarfForm;
using dwarf2reader::DwarfTag;

class IDwarfReader {
// All offsets are from the beginning of the .debug_info for this module
// and when combined with the module uniquely identify the object.  In
// addition all notifications function which can have children return a
// context as a void *, which be passed into any child references to
// facility building of tree structure.  The order of calls and the values
 // they pass can be seen bellow:
//
// root = StartCompilationUnit(...)
//  child = StartDIE(root, ....)
//            Property(child, ....)
//   sub    = StartDIE(child, ...)
//              Property(sub, ....)
//            EndDIE(sub)
//          EndDIE(child)
//          AddDir(root, ...)
//          AddFile(root, ...)
//          AddLine(root, ...)
//        EndComplicationUnit(root)

 public:
  // Start to process a compilation unit at OFFSET from the beginning of the
  // .debug_info section. Return false if you would like to skip this
  // compilation unit.
  virtual void *StartCompilationUnit(uint64 offset, uint8 address_size,
                                    uint8 offset_size, uint64 cu_length,
                                    uint8 dwarf_version) = 0;

  // Called when finished processing the CompilationUnit at OFFSET.
  // CU's are in the form of a linear list, so each one forms the root of
  // a DIE tree.
  virtual void EndCompilationUnit(void *ctx, uint64 offset) = 0;

  // Start to process a DIE at OFFSET from the beginning of the .debug_info
  // section. Return false if you would like to skip this DIE.
  virtual void *StartDIE(void *ctx, uint64 parent, uint64 offset, enum DwarfTag tag) = 0;

  // Called when finished processing the DIE at OFFSET.
  // Because DWARF2/3 specifies a tree of DIEs, you may get starts
  // before ends of the previous DIE, as we process children before
  // ending the parent.
  virtual void EndDIE(void *ctx, uint64 offset) = 0;

  // Called when we have an attribute with unsigned data to give to our
  // handler. The attribute is for the DIE at OFFSET from the beginning of the
  // .debug_info section. Its name is ATTR, its form is FORM, and its value is
  // DATA.
  virtual void ProcessAttributeUnsigned(void *ctx,
                                        uint64 offset,
                                        uint64 parent,
                                        enum DwarfAttribute attr,
                                        enum DwarfForm form,
                                        uint64 data) = 0;

  // Called when we have an attribute with signed data to give to our handler.
  // The attribute is for the DIE at OFFSET from the beginning of the
  // .debug_info section. Its name is ATTR, its form is FORM, and its value is
  // DATA.
  virtual void ProcessAttributeSigned(void *ctx,
                                      uint64 offset,
                                      uint64 parent,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      int64 data) = 0;

  // Called when we have an attribute whose value is a reference to
  // another DIE. The attribute belongs to the DIE at OFFSET from the
  // beginning of the .debug_info section. Its name is ATTR, its form
  // is FORM, and the offset of the DIE being referred to from the
  // beginning of the .debug_info section is DATA.
  virtual void ProcessAttributeReference(void *ctx,
                                         uint64 offset,
                                         uint64 parent,
                                         enum DwarfAttribute attr,
                                         enum DwarfForm form,
                                         uint64 data) = 0;

  // Called when we have an attribute with a buffer of data to give to our
  // handler. The attribute is for the DIE at OFFSET from the beginning of the
  // .debug_info section. Its name is ATTR, its form is FORM, DATA points to
  // the buffer's contents, and its length in bytes is LENGTH. The buffer is
  // owned by the caller, not the callee, and may not persist for very long.
  // If you want the data to be available later, it needs to be copied.
  virtual void ProcessAttributeBuffer(void *ctx,
                                      uint64 offset,
                                      uint64 parent,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const char* data,
                                      uint64 len) = 0;

  // Called when we have an attribute with string data to give to our handler.
  // The attribute is for the DIE at OFFSET from the beginning of the
  // .debug_info section. Its name is ATTR, its form is FORM, and its value is
  // DATA.
  virtual void ProcessAttributeString(void *ctx,
                                      uint64 offset,
                                      uint64 parent,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const char* data) = 0;

  // Called when we define a directory.  NAME is the directory name,
  // DIR_NUM is one based directory index number.  The context will
  // be the one passe back by StartCompileUnit.
  virtual void DefineDir(void *ctx, const char *name, uint32 dir_num) = 0;

  // Called when we define a file.  NAME is the directory name,
  // DIR_NUM is one based directory index number.
  virtual void DefineFile(void *ctx, const char *name, int32 file_num,
                          uint32 dir_num, uint64 mod_time,
                          uint64 length) = 0;

  // Called when the line info reader has a new line, address pair
  // ready for us. ADDRESS is the address of the code, LENGTH is the
  // length of its machine code in bytes, FILE_NUM is the file number
  // containing the code, LINE_NUM is the line number in that file for
  // the code, and COLUMN_NUM is the column number the code starts at,
  // if we know it (0 otherwise).
  virtual void AddLine(void *ctx, uint64 address, uint64 length, uint32 file_num,
                       uint32 line_num, uint32 column_num) = 0;

  // Called when the location list reader has a new location list entry
  // ready for us. 
  // Params:
  //    offset:         offset of this entry from the beginning of the 
  //                    .debug_loc section. This is used elsewhere to identify
  //                    the location list.
  //
  //    is_first_entry: If true, signifies that this entry is the first entry
  //                    in a new list. The DWARF info refers to lists by their
  //                    first entry, so if is_first_entry is false it's not
  //                    particularly important to remember the value of offset.
  //
  //    lowPc, highPc:  Range of program counter values for which this location
  //                    entry is valid.
  //
  //    data, dataSize: Defines an array of bytes containg DWARF VM
  //                    instructions that can be used to decode the address
  //                    encoded in this loc entry.
  virtual void AddLocListEntry(
    uint64 offset,
    bool is_first_entry,
    uint64 lowPc,
    uint64 highPc,
    const void* data,
    size_t dataSize) = 0;

  //
  // Call Frame Information (CFI) handling
  //

  enum CFI_RuleType {
    CFIRT_UNDEFINED,
    CFIRT_SAMEVALUE,
    CFIRT_OFFSET,
    CFIRT_VALOFFSET,
    CFIRT_REGISTER,
    CFIRT_EXPRESSION,
    CFIRT_VALEXPRESSION
  };

  virtual bool BeginCfiEntry (
    size_t offset, 
    uint64 address, 
    uint64 length,
    uint8 version, 
    const char* augmentation,
    unsigned return_address) = 0;

  virtual bool AddCfiRule(
    uint64 address,
    int reg,
    CFI_RuleType ruleType,
    int base_register,
    int32 offset,
    const void* expression,
    uint32 expressionLength) = 0;

  virtual bool EndCfiEntry() = 0;
};


}  // namespace dwarf_reader

#endif  // DWARF_READER_DWARF_READER_H_

