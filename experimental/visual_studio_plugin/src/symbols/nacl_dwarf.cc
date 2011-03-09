#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include <algorithm>
#include <cassert>
#include <string>
#include <sstream>

#define __LIBELF_INTERNAL__ 1
#include "private.h"
//#include "libelf.h"
//#include "gelf.h"

#define ElfW(x) Elf64_##x

#include "nacl_symbols.h"
#include "nacl_symbols_impl.h"
#include "common\dwarf\dwarf2reader.h"
#include "common\dwarf_cu_to_module.h"
#include "common\dwarf_line_to_module.h"

using namespace google_breakpad;

// A line-to-module loader that accepts line number info parsed by
// dwarf2reader::LineInfo and populates a Module and a line vector
// with the results.
namespace google_breakpad {


class DumperLineToModule: public DwarfCUToModule::LineToModuleFunctor {
 public:
  // Create a line-to-module converter using BYTE_READER.
  DumperLineToModule(dwarf2reader::ByteReader *byte_reader)
      : byte_reader_(byte_reader) { }
  void operator()(const char *program, uint64 length,
                  Module *module, vector<Module::Line> *lines) {
    DwarfLineToModule handler(module, lines);
    dwarf2reader::LineInfo parser(program, length, byte_reader_, &handler);
    parser.Start();
  }
 private:
  dwarf2reader::ByteReader *byte_reader_;
};

} /* End of google_breakpad Namespace */

namespace NaCl {

bool NaClLoadDwarf(SymbolMapImpl *impl) {
  ElfW(Ehdr) *elf_header = (Elf64_Ehdr *) impl->GetElf()->e_data;

  dwarf2reader::Endianness endianness;
  if (elf_header->e_ident[EI_DATA] == ELFDATA2LSB)
    endianness = dwarf2reader::ENDIANNESS_LITTLE;
  else if (elf_header->e_ident[EI_DATA] == ELFDATA2MSB)
    endianness = dwarf2reader::ENDIANNESS_BIG;
  else {
    printf("Bad data encoding in ELF header\n");
    return false;
  }
  dwarf2reader::ByteReader byte_reader(endianness);


  // Parse all the compilation units in the .debug_info section.
  DumperLineToModule line_to_module(&byte_reader);
  std::pair<const char *, uint64> debug_info_section
      = impl->GetFileContext()->section_map[".debug_info"];
  // We should never have been called if the file doesn't have a
  // .debug_info section.
  assert(debug_info_section.first);
  uint64 debug_info_length = debug_info_section.second;
  for (uint64 offset = 0; offset < debug_info_length;) {
    // Make a handler for the root DIE that populates MODULE with the
    // data we find.
    DwarfCUToModule::WarningReporter reporter(impl->GetFileContext()->filename, offset);
    DwarfCUToModule root_handler(impl->GetFileContext(), &line_to_module, &reporter);
    // Make a Dwarf2Handler that drives our DIEHandler.
    dwarf2reader::DIEDispatcher die_dispatcher(&root_handler);
    // Make a DWARF parser for the compilation unit at OFFSET.
    dwarf2reader::CompilationUnit reader(impl->GetFileContext()->section_map,
                                         offset,
                                         &byte_reader,
                                         &die_dispatcher);
    // Process the entire compilation unit; get the offset of the next.
    offset += reader.Start();
  }

  // Time to number the files we got
  vector<MapModule::File *> files;
  impl->GetModule()->GetFiles(&files);
  for (uint32_t loop = 0; loop < files.size(); loop++) 
    files[loop]->source_id = loop;

  return true;
}

} /* End of NaCl Namespace */

