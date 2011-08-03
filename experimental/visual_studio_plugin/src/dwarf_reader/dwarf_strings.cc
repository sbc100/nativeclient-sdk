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

#include <assert.h>

#include <map>

#include "common/dwarf/dwarf2enums.h"
#include "common/dwarf/types.h"

#include "dwarf_reader/dwarf_strings.h"


static int32 s_DwarfStringsInit = 0;

static std::map<uint32,const char *> s_FormMap;
static std::map<uint32,const char *> s_TagMap;
static std::map<uint32,const char *> s_AttributeMap;

#define SET_TAG(x)  s_TagMap[DW_TAG_##x] = #x;
#define SET_ATTR(x) s_AttributeMap[DW_AT_##x] = #x;
#define SET_FORM(x) s_FormMap[DW_FORM_##x] = #x;


using namespace dwarf2reader;

namespace dwarf_reader {

static void BuildTagLookup() {
  SET_TAG(padding)
  SET_TAG(array_type)
  SET_TAG(class_type)
  SET_TAG(entry_point)
  SET_TAG(enumeration_type)
  SET_TAG(formal_parameter)
  SET_TAG(imported_declaration)
  SET_TAG(label)
  SET_TAG(lexical_block)
  SET_TAG(member)
  SET_TAG(pointer_type)
  SET_TAG(reference_type)
  SET_TAG(compile_unit)
  SET_TAG(string_type)
  SET_TAG(structure_type)
  SET_TAG(subroutine_type)
  SET_TAG(typedef)
  SET_TAG(union_type)
  SET_TAG(unspecified_parameters)
  SET_TAG(variant)
  SET_TAG(common_block)
  SET_TAG(common_inclusion)
  SET_TAG(inheritance)
  SET_TAG(inlined_subroutine)
  SET_TAG(module)
  SET_TAG(ptr_to_member_type)
  SET_TAG(set_type)
  SET_TAG(subrange_type)
  SET_TAG(with_stmt)
  SET_TAG(access_declaration)
  SET_TAG(base_type)
  SET_TAG(catch_block)
  SET_TAG(const_type)
  SET_TAG(constant)
  SET_TAG(enumerator)
  SET_TAG(file_type)
  SET_TAG(friend)
  SET_TAG(namelist)
  SET_TAG(namelist_item)
  SET_TAG(packed_type)
  SET_TAG(subprogram)
  SET_TAG(template_type_param)
  SET_TAG(template_value_param)
  SET_TAG(thrown_type)
  SET_TAG(try_block)
  SET_TAG(variant_part)
  SET_TAG(variable)
  SET_TAG(volatile_type)
  // DWARF 3.
  SET_TAG(dwarf_procedure)
  SET_TAG(restrict_type)
  SET_TAG(interface_type)
  SET_TAG(namespace)
  SET_TAG(imported_module)
  SET_TAG(unspecified_type)
  SET_TAG(partial_unit)
  SET_TAG(imported_unit)
  // SGI/MIPS Extensions.
  SET_TAG(MIPS_loop)
  // HP extensions.  See:
  // ftp://ftp.hp.com/pub/lang/tools/WDB/wdb-4.0.tar.gz
  SET_TAG(HP_array_descriptor)
  // GNU extensions.
  SET_TAG(format_label)
  SET_TAG(function_template)
  SET_TAG(class_template)
  SET_TAG(GNU_BINCL)
  SET_TAG(GNU_EINCL)
  // Extensions for UPC.  See: http://upc.gwu.edu/~upc.
  SET_TAG(upc_shared_type)
  SET_TAG(upc_strict_type)
  SET_TAG(upc_relaxed_type)
  // PGI (STMicroelectronics) extensions.  No documentation available.
  SET_TAG(PGI_kanji_type)
  SET_TAG(PGI_interface_block)
}

static void BuildAttributeLookup() {
  SET_ATTR(sibling)
  SET_ATTR(location)
  SET_ATTR(name)
  SET_ATTR(ordering)
  SET_ATTR(subscr_data)
  SET_ATTR(byte_size)
  SET_ATTR(bit_offset)
  SET_ATTR(bit_size)
  SET_ATTR(element_list)
  SET_ATTR(stmt_list)
  SET_ATTR(low_pc)
  SET_ATTR(high_pc)
  SET_ATTR(language)
  SET_ATTR(member)
  SET_ATTR(discr)
  SET_ATTR(discr_value)
  SET_ATTR(visibility)
  SET_ATTR(import)
  SET_ATTR(string_length)
  SET_ATTR(common_reference)
  SET_ATTR(comp_dir)
  SET_ATTR(const_value)
  SET_ATTR(containing_type)
  SET_ATTR(default_value)
  SET_ATTR(inline)
  SET_ATTR(is_optional)
  SET_ATTR(lower_bound)
  SET_ATTR(producer)
  SET_ATTR(prototyped)
  SET_ATTR(return_addr)
  SET_ATTR(start_scope)
  SET_ATTR(stride_size)
  SET_ATTR(upper_bound)
  SET_ATTR(abstract_origin)
  SET_ATTR(accessibility)
  SET_ATTR(address_class)
  SET_ATTR(artificial)
  SET_ATTR(base_types)
  SET_ATTR(calling_convention)
  SET_ATTR(count)
  SET_ATTR(data_member_location)
  SET_ATTR(decl_column)
  SET_ATTR(decl_file)
  SET_ATTR(decl_line)
  SET_ATTR(declaration)
  SET_ATTR(discr_list)
  SET_ATTR(encoding)
  SET_ATTR(external)
  SET_ATTR(frame_base)
  SET_ATTR(friend)
  SET_ATTR(identifier_case)
  SET_ATTR(macro_info)
  SET_ATTR(namelist_items)
  SET_ATTR(priority)
  SET_ATTR(segment)
  SET_ATTR(specification)
  SET_ATTR(static_link)
  SET_ATTR(type)
  SET_ATTR(use_location)
  SET_ATTR(variable_parameter)
  SET_ATTR(virtuality)
  SET_ATTR(vtable_elem_location)
  // DWARF 3 values.
  SET_ATTR(allocated)
  SET_ATTR(associated)
  SET_ATTR(data_location)
  SET_ATTR(stride)
  SET_ATTR(entry_pc)
  SET_ATTR(use_UTF8)
  SET_ATTR(extension)
  SET_ATTR(ranges)
  SET_ATTR(trampoline)
  SET_ATTR(call_column)
  SET_ATTR(call_file)
  SET_ATTR(call_line)
  // SGI/MIPS extensions.
  SET_ATTR(MIPS_fde)
  SET_ATTR(MIPS_loop_begin)
  SET_ATTR(MIPS_tail_loop_begin)
  SET_ATTR(MIPS_epilog_begin)
  SET_ATTR(MIPS_loop_unroll_factor)
  SET_ATTR(MIPS_software_pipeline_depth)
  SET_ATTR(MIPS_linkage_name)
  SET_ATTR(MIPS_stride)
  SET_ATTR(MIPS_abstract_name)
  SET_ATTR(MIPS_clone_origin)
  SET_ATTR(MIPS_has_inlines)
  // HP extensions.
  SET_ATTR(HP_block_index)
  SET_ATTR(HP_unmodifiable)
  SET_ATTR(HP_actuals_stmt_list)
  SET_ATTR(HP_proc_per_section)
  SET_ATTR(HP_raw_data_ptr)
  SET_ATTR(HP_pass_by_reference)
  SET_ATTR(HP_opt_level)
  SET_ATTR(HP_prof_version_id)
  SET_ATTR(HP_opt_flags)
  SET_ATTR(HP_cold_region_low_pc )
  SET_ATTR(HP_cold_region_high_pc)
  SET_ATTR(HP_all_variables_modifiable)
  SET_ATTR(HP_linkage_name)
  SET_ATTR(HP_prof_flags)
  // GNU extensions.
  SET_ATTR(sf_names)
  SET_ATTR(src_info)
  SET_ATTR(mac_info)
  SET_ATTR(src_coords)
  SET_ATTR(body_begin)
  SET_ATTR(body_end)
  SET_ATTR(GNU_vector)
  // VMS extensions.
  SET_ATTR(VMS_rtnbeg_pd_address)
  // UPC extension.
  SET_ATTR(upc_threads_scaled)
  // PGI (STMicroelectronics) extensions.
  SET_ATTR(PGI_lbase)
  SET_ATTR(PGI_soffset)
  SET_ATTR(PGI_lstride)
}

static void BuildFormLookup() {
// Form names and codes.
  SET_FORM(addr)
  SET_FORM(block2)
  SET_FORM(block4)
  SET_FORM(data2)
  SET_FORM(data4)
  SET_FORM(data8)
  SET_FORM(string)
  SET_FORM(block)
  SET_FORM(block1)
  SET_FORM(data1)
  SET_FORM(flag)
  SET_FORM(sdata)
  SET_FORM(strp)
  SET_FORM(udata)
  SET_FORM(ref_addr)
  SET_FORM(ref1)
  SET_FORM(ref2)
  SET_FORM(ref4)
  SET_FORM(ref8)
  SET_FORM(ref_udata)
  SET_FORM(indirect)
}


static inline void BuildLookups() {
  if (!s_DwarfStringsInit) {
     s_DwarfStringsInit = 1;
     BuildTagLookup();
     BuildFormLookup();
     BuildAttributeLookup();
  }
}


const char *DwarfAttributeName(enum DwarfAttribute attr) {
   const char *out;

   BuildLookups();

   if (attr == DW_AT_data_member_location)
     out = 0;

   out = s_AttributeMap[attr];
   assert(NULL != out);
   return out;
}

const char *DwarfTagName(enum DwarfTag tag) {
   const char *out;

   BuildLookups();

   out = s_TagMap[tag];
   assert(NULL != out);
   return out;
}


const char *DwarfFormName(enum DwarfForm form) {
  const char *out;

  BuildLookups();

  out = s_FormMap[form];
  assert(NULL != out);
  return out;
}


}  // namespace dwarf2reader

