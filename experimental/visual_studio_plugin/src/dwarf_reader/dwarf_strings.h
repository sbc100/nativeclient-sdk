// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// This file defines the ElfObject which decodes the elf data and calls
// the appropriate reader callbacks.  The reader interface allows the
// libary user to examine the ELF file without ever needing to know
// machine class (32/64 bit).

#ifndef DWARF_READER_DWARF_STRINGS_H_
#define DWARF_READER_DWARF_STRINGS_H_

#include "common/types.h"
#include "common/dwarf/dwarf2enums.h"

using dwarf2reader::DwarfAttribute;
using dwarf2reader::DwarfTag;
using dwarf2reader::DwarfForm;


namespace dwarf_reader {

const char *DwarfAttributeName(enum DwarfAttribute attr);
const char *DwarfTagName(enum DwarfTag tag);
const char *DwarfFormName(enum DwarfForm form);

}  // namespace dwarf2reader
#endif  // DWARF_READER_DWARF_STRINGS_H__

