// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DWARF_READER_DWARF_READER_H_
#define DWARF_READER_DWARF_READER_H_

#include "common/types.h"
#include "common/dwarf/dwarf2enums.h"

namespace dwarf_reader {

using dwarf2reader::DwarfAttribute;
using dwarf2reader::DwarfForm;
using dwarf2reader::DwarfTag;

/// The interface for processing DWARF Data
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
// TODO(mlinck) remove unused ctx args and return values from this interface.
 public:
  /// Call Frame Information (CFI) handling
  enum CFI_RuleType {
    CFIRT_UNDEFINED,
    CFIRT_SAMEVALUE,
    CFIRT_OFFSET,
    CFIRT_VALOFFSET,
    CFIRT_REGISTER,
    CFIRT_EXPRESSION,
    CFIRT_VALEXPRESSION
  };

  /// Start to process a compilation unit at OFFSET from the beginning of the
  /// .debug_info section. Return false if you would like to skip this
  /// compilation unit.
  /// @param[in] offset The offset of this compilation unit in its file
  /// segment.
  /// @param[in] address_size The size of an address on this architecture.
  /// @param[in] offset_size The size of an offset on this architecture.
  /// @param[in] cu_length The length of the compilation unit entry.
  /// @param[in] dwarf_version The version of the dwarf_spec being used.
  /// @return a context to be handed to the next call to this function (if
  /// used.)
  virtual void *StartCompilationUnit(uint64 offset,
                                     uint8 address_size,
                                     uint8 offset_size,
                                     uint64 cu_length,
                                     uint8 dwarf_version) = 0;

  /// Called when finished processing the CompilationUnit at OFFSET.
  /// CU's are in the form of a linear list, so each one forms the root of
  /// a DIE tree. TODO(mlinck) it may be possible to get rid of this function.
  /// @param[in] ctx This argument is not used in the SymbolDatabase
  /// @param[in] ctx The offset identifies which compilation unit is being
  /// ended.
  virtual void EndCompilationUnit(void *ctx, uint64 offset) = 0;

  /// Start to process a DIE at |offset| from the beginning of the .debug_info
  /// section. Return false if you would like to skip this DIE.
  /// @param[in] ctx Context returned by the previous call to StartDIE (if
  /// used.)
  /// @param[in] parent The offset of this DIE's parent (or 0)
  /// @param[in] offset This DIE's offset in the .debug_info section.
  /// @param[in] tag This DIE's type descriptor.
  /// @return A context that may be passed to this DIE's children (if used.)
  virtual void *StartDIE(void *ctx,
                         uint64 parent,
                         uint64 offset,
                         enum DwarfTag tag) = 0;

  /// Called when finished processing the DIE at OFFSET.
  /// Because DWARF2/3 specifies a tree of DIEs, you may get starts
  /// before ends of the previous DIE, as we process children before
  /// ending the parent.
  /// @param[in] ctx The context returned when this DIE was started (if used.)
  /// @param[in] offset The offset of this DIE'd end-point in the .debug_info
  /// section.
  virtual void EndDIE(void *ctx, uint64 offset) = 0;

  /// Called when we have an attribute with unsigned data to give to our
  /// handler. The attribute is for the DIE at OFFSET from the beginning of the
  /// .debug_info section. Its name is ATTR, its form is FORM, and its value is
  /// DATA.
  /// @param[in] ctx To be removed.
  /// @param[in] offset The offset of this attribute in the .debug_info
  /// section.
  /// @param[in] parent The DIE modified by this attribute.
  /// @param[in] attr The type specifier for this attribute.
  /// @param[in] form The form of data that is associated with this attribute.
  /// @param[in] data The actual data.
  virtual void ProcessAttributeUnsigned(void *ctx,
                                        uint64 offset,
                                        uint64 parent,
                                        enum DwarfAttribute attr,
                                        enum DwarfForm form,
                                        uint64 data) = 0;

  /// Called when we have an attribute with signed data to give to our handler.
  /// The attribute is for the DIE at OFFSET from the beginning of the
  /// .debug_info section. Its name is ATTR, its form is FORM, and its value is
  /// DATA.
  /// @param[in] ctx To be removed.
  /// @param[in] offset The offset of this attribute in the .debug_info
  /// section.
  /// @param[in] parent The DIE modified by this attribute.
  /// @param[in] attr The type specifier for this attribute.
  /// @param[in] form The form of data that is associated with this attribute.
  /// @param[in] data The actual data.
  virtual void ProcessAttributeSigned(void *ctx,
                                      uint64 offset,
                                      uint64 parent,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      int64 data) = 0;

  /// Called when we have an attribute whose value is a reference to
  /// another DIE. The attribute belongs to the DIE at OFFSET from the
  /// beginning of the .debug_info section. Its name is ATTR, its form
  /// is FORM, and the offset of the DIE being referred to from the
  /// beginning of the .debug_info section is DATA.
  /// @param[in] ctx To be removed.
  /// @param[in] offset The offset of this attribute in the .debug_info
  /// section.
  /// @param[in] parent The DIE modified by this attribute.
  /// @param[in] attr The type specifier for this attribute.
  /// @param[in] form The form of data that is associated with this attribute.
  /// @param[in] data The actual data.
  virtual void ProcessAttributeReference(void *ctx,
                                         uint64 offset,
                                         uint64 parent,
                                         enum DwarfAttribute attr,
                                         enum DwarfForm form,
                                         uint64 data) = 0;

  /// Called when we have an attribute with a buffer of data to give to our
  /// handler. The attribute is for the DIE at OFFSET from the beginning of the
  /// .debug_info section. Its name is ATTR, its form is FORM, DATA points to
  /// the buffer's contents, and its length in bytes is LENGTH. The buffer is
  /// owned by the caller, not the callee, and may not persist for very long.
  /// If you want the data to be available later, it needs to be copied.
  /// @param[in] ctx To be removed.
  /// @param[in] offset The offset of this attribute in the .debug_info
  /// section.
  /// @param[in] parent The DIE modified by this attribute.
  /// @param[in] attr The type specifier for this attribute.
  /// @param[in] form The form of data that is associated with this attribute.
  /// @param[in] data The actual data.
  virtual void ProcessAttributeBuffer(void *ctx,
                                      uint64 offset,
                                      uint64 parent,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const char* data,
                                      uint64 len) = 0;

  /// Called when we have an attribute with string data to give to our handler.
  /// The attribute is for the DIE at OFFSET from the beginning of the
  /// .debug_info section. Its name is ATTR, its form is FORM, and its value is
  /// DATA.
  /// @param[in] ctx To be removed.
  /// @param[in] offset The offset of this attribute in the .debug_info
  /// section.
  /// @param[in] parent The DIE modified by this attribute.
  /// @param[in] attr The type specifier for this attribute.
  /// @param[in] form The form of data that is associated with this attribute.
  /// @param[in] data The actual data.
  virtual void ProcessAttributeString(void *ctx,
                                      uint64 offset,
                                      uint64 parent,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const char* data) = 0;

  /// Called when we define a directory.
  /// @param[in] ctx Whatever was passed back by |StartCompileUnit|
  /// @param[in] name The name of the directory.
  /// @param[in] dir_num The directory handle.
  virtual void DefineDir(void *ctx, const char *name, uint32 dir_num) = 0;

  /// Called when we define a file.
  /// @param[in] ctx Whatever was passed back by |StartCompileUnit|
  /// @param[in] name The file name.
  /// @param[in] file_num The file handle.
  /// @param[in] dir_num The file's parent directory's handle.
  /// @param[in] mod_time The last time the file was modified.
  /// @param[in] length The file's length.
  virtual void DefineFile(void *ctx,
                          const char *name,
                          int32 file_num,
                          uint32 dir_num,
                          uint64 mod_time,
                          uint64 length) = 0;

  /// Called when the line info reader has a new line, address pair
  /// ready for us.
  /// @param[in] ctx Whatever was passed back by |StartCompileUnit|
  /// @param[in] address The offset of the code relative of the start of the
  /// .debug_line section of the binary.
  /// @param[in] length The length of its machine code in bytes.
  /// @param[in] file_num The file number containing the code.
  /// @param[in] line_num The line number in that file for the code.
  /// @param[in] column_num The column number the code starts at if we know,
  /// 0 otherwise.
  virtual void AddLine(void *ctx,
                       uint64 address,
                       uint64 length,
                       uint32 file_num,
                       uint32 line_num,
                       uint32 column_num) = 0;

  /// Called when the location list reader has a new location list entry
  /// ready for us.
  /// @param[in] offset The offset of this entry from the beginning of the
  /// .debug_loc section. This is used elsewhere to identify the location
  /// list.
  /// @param[in] is_first_entry If true, signifies that this entry is the
  /// first entry in a new list. The DWARF info refers to lists by their
  /// first entry, so if is_first_entry is false it's not particularly
  /// important to remember the value of offset.
  /// @param[in] low_pc Beginning of the range of program counter values for
  /// which this location entry is valid.
  /// @param[in] high_pc End of the range of program counter values for which
  /// this location entry is valid.
  /// @param[in] data The beginning of an array of bytes containg DWARF VM
  /// instructions that can be used to decode the address encoded in this loc
  /// entry.
  /// @param[in] dataSize The size of the array of bytes.
  virtual void AddLocListEntry(uint64 offset,
                               bool is_first_entry,
                               uint64 lowPc,
                               uint64 highPc,
                               const void* data,
                               size_t dataSize) = 0;

  /// Starts a new Call Frame Information entry (CFI) in the reader.  Call
  /// Frame information entries allow the debugger to perform manipulations on
  /// the call state of the program beign debugged, for example to reproduce
  /// the state of the stack at a particular point during execution.
  /// @param[in] offset The offset of the CFI relative to the beginning of the
  /// .eh_frame section of the binary.
  /// @param[in] address The address of the call frame being modified.
  /// @param[in] length The length of the CFI entry.
  /// @param[in] version The version of the CFI entry (this area of the spec
  /// changes)
  /// @param[in] augmentation The augmentation to be performed on the call
  /// frame as required by the architecture (See section 6.4.1 of the DWARF 3
  /// spec)
  /// @param[in] return_address The inex of the register that holds the return
  /// address from the call frame being described.
  /// @return |true| if the operation was successful.
  virtual bool BeginCfiEntry(size_t offset,
                             uint64 address,
                             uint64 length,
                             uint8 version,
                             const char* augmentation,
                             unsigned return_address) = 0;

  /// Starts a new Call Frame rule.  For a description of call frame rules,
  /// please read dwarf2reader.h and DWARF spec 3, section 6.4.2.3
  /// @param[in] address The address at which this Rule becomes applicable.
  /// It becomes inapplicable when overridden by a rule for the same register
  /// at a higher address.
  /// @param[in] reg The register to which this rule applies.
  /// @param[in] rule_type The type of rule (Undefined, Same Value, Offset,
  /// Value Offset, Register, Expression or Value Expression)
  /// @param[in] base_register If the type of the rule is any kind of offset
  /// rule, then the offset is relative to the content of this register.
  /// @param[in] offset The offset, to be applied as part of this rule, if an
  /// offset rule.
  /// @param[in] expression The expression to be used if this is an Expression
  /// or a Value Expression rule.
  /// @param[in] expression_length The size of the expression.
  /// @return |true| of the operation was successful.
  virtual bool AddCfiRule(uint64 address,
                          int reg,
                          CFI_RuleType rule_type,
                          int base_register,
                          int32 offset,
                          const void* expression,
                          uint32 expression_length) = 0;

  /// Notifies the reader that the end of a call frame information entry has
  /// been reached.
  /// @return |true| of the operation was successful.
  virtual bool EndCfiEntry() = 0;

  /// Range List handling.
  /// @param[in] offset The offset of this entry's range list from the
  /// beginning of the .debug_ranges section of the binary.
  /// @param[in] base_address The base address to be applied to this entry.
  /// If this is 0xFFFFFFFF for 32 bit or 0xFFFFFFFFFFFFFFFF for 64 bit, then
  /// the Compilation Unit's base is to be used instead.
  /// @param[in] low_pc The beginning of the range described by this entry.
  /// @param[in] high_pc The end of the range described by this entry.
  virtual void AddRangeListEntry(uint64 offset,
                                 uint64 base_address,
                                 uint64 low_pc,
                                 uint64 high_pc) = 0;
};

}  // namespace dwarf_reader

#endif  // DWARF_READER_DWARF_READER_H_

