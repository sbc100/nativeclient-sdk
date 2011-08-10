// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dwarf_reader/dwarf_info_parser.h"
#include "dwarf_reader/dwarf_reader.h"
#include "dwarf_reader/parse_state.h"

namespace dwarf_reader {

DwarfInfoParser::DwarfInfoParser(dwarf_reader::ParseState *parse_state,
                                 IDwarfReader *reader)
  :  parse_state_(parse_state),
     reader_(reader) {}

bool DwarfInfoParser::StartCompilationUnit(uint64 offset,
                                           uint8 address_size,
                                           uint8 offset_size,
                                           uint64 compilation_unit_length,
                                           uint8 dwarf_version) {
  void *context = reader_->StartCompilationUnit(
      offset,
      address_size,
      offset_size,
      compilation_unit_length,
      dwarf_version);

  parse_state_->set_current_compilation_unit(context);
  parse_state_->PushStackFrame(context, offset);
  return true;
}

bool DwarfInfoParser::StartDIE(
    uint64 offset,
    enum DwarfTag tag,
    const dwarf2reader::AttributeList& attributes) {
  void *context = reader_->StartDIE(
      parse_state_->GetTopStackContext(),
      parse_state_->GetTopStackAddress(),
      offset,
      tag);
  parse_state_->PushStackFrame(context, offset);
  return true;
}

void DwarfInfoParser::EndDIE(uint64 offset) {
  reader_->EndDIE(parse_state_->GetTopStackContext(),
                  parse_state_->GetTopStackAddress());
  parse_state_->PopStackFrame();
}

void DwarfInfoParser::ProcessAttributeUnsigned(
    uint64 offset,
    enum DwarfAttribute attribute,
    enum DwarfForm form,
    uint64 data) {
  reader_->ProcessAttributeUnsigned(
      parse_state_->GetTopStackContext(),
      parse_state_->GetTopStackAddress(),
      offset,
      attribute,
      form,
      data);
}

void DwarfInfoParser::ProcessAttributeSigned(
    uint64 offset,
    enum DwarfAttribute attribute,
    enum DwarfForm form,
    int64 data) {
  reader_->ProcessAttributeSigned(
      parse_state_->GetTopStackContext(),
      parse_state_->GetTopStackAddress(),
      offset,
      attribute,
      form,
      data);
}

void DwarfInfoParser::ProcessAttributeReference(
  uint64 offset,
  enum DwarfAttribute attribute,
  enum DwarfForm form,
  uint64 data) {
  reader_->ProcessAttributeReference(
      parse_state_->GetTopStackContext(),
      parse_state_->GetTopStackAddress(),
      offset,
      attribute,
      form,
      data);
}

void DwarfInfoParser::ProcessAttributeBuffer(uint64 offset,
                                             enum DwarfAttribute attribute,
                                             enum DwarfForm form,
                                             const char* data,
                                             uint64 len) {
  reader_->ProcessAttributeBuffer(
      parse_state_->GetTopStackContext(),
      parse_state_->GetTopStackAddress(),
      offset,
      attribute,
      form,
      data,
      len);
}

void DwarfInfoParser::ProcessAttributeString(uint64 offset,
                                             enum DwarfAttribute attribute,
                                             enum DwarfForm form,
                                             const string& data) {
  reader_->ProcessAttributeString(
      parse_state_->GetTopStackContext(),
      parse_state_->GetTopStackAddress(),
      offset,
      attribute,
      form,
      data.data());
}

}  // namespace dwarf_reader
