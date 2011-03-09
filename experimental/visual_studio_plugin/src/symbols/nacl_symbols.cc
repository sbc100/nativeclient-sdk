#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#include <algorithm>
#include <sstream>

#define __LIBELF_INTERNAL__ 1
#include "libelf.h"
#include "gelf.h"

#include "nacl_symbols.h"
#include "nacl_symbols_impl.h"
#include "common\dwarf_cu_to_module.h"
#include "common\dwarf_line_to_module.h"

#include "nacl_xml_helpers.h"

using namespace std;



extern "C" char *elf_getedata(Elf *elf);

namespace NaCl {

extern bool NaClLoadDwarf(SymbolMapImpl *impl);

ISymbolMap *SymbolMapFactory(const char *file) {
  SymbolMapImpl *map = 0;
  struct stat elf_stats;	// fstat struct
  int fd;   
  stringstream ostr;


  if (0 == file)
    return NULL;

  if((fd = _open(file, _O_RDONLY | _O_BINARY)) == -1)
  {
		  printf("SymbolbMap could not open %s\n", file);
		  goto failed;
  }

  if(fstat(fd, &elf_stats))
  {
    printf("SymbolbMap could not fstat %s\n", file);
    goto failed;
  }

  /* Check libelf version first */
  if(elf_version(EV_CURRENT) == EV_NONE)
	  printf("WARNING Elf Library is out of date!\n");

  Elf *elf = elf_begin(fd, ELF_C_READ, NULL);	// Initialize 'elf' pointer to our file descriptor

  if (elf)
    map = new SymbolMapImpl(fd, elf, file);


  Elf_Scn *scn = 0;
  size_t strsec = 0;
  size_t offset = 0;

  elf_getshdrstrndx(elf, &strsec);
  offset = (size_t) elf_getedata(elf);

  XMLAddTagStart(ostr, 0, "vars");

  while((scn = elf_nextscn(elf, scn)) != 0) {
    GElf_Shdr shdr;
    Elf_Data *edata = 0;
    const char *name;
    const char *contents;
    uint64 length;



    // Store the section headers
    gelf_getshdr(scn, &shdr);
    name = elf_strptr(elf, strsec, shdr.sh_name);
    contents = reinterpret_cast<const char *>(shdr.sh_offset);
    length = shdr.sh_size;
    contents += (uint64_t) offset;
    map->GetFileContext()->section_map[name] = std::make_pair(contents, length);

	  // When we find a section header marked SHT_SYMTAB stop and get symbols
	  if(shdr.sh_type == SHT_SYMTAB)
    {
		  // edata points to our symbol table
		  edata = elf_getdata(scn, edata);

		  // how many symbols are there? this number comes from the size of
		  // the section divided by the entry size
		  uint32_t symbol_count = shdr.sh_size / shdr.sh_entsize;
      uint32_t i;
      GElf_Sym sym;

		  // loop through to grab all symbols
		  for(i = 0; i < symbol_count; i++)
      {			
			  // libelf grabs the symbol data using gelf_getsym()
        gelf_getsym(edata, i, &sym);

        if (ELF32_ST_TYPE(sym.st_info) != STT_OBJECT)
          continue;

        XMLAddTagStart(ostr, 1, "var");
        XMLAddString(ostr, 2, "name", elf_strptr(elf, shdr.sh_link, sym.st_name));
        XMLAddPointer(ostr,2, "addr", sym.st_value);
        XMLAddNumber(ostr, 2, "size", sym.st_size);
  	
			  // type of symbol binding
			  switch(ELF32_ST_BIND(sym.st_info))
			  {
				  case STB_LOCAL:  XMLAddString(ostr, 2, "binding", "LOCAL"); break;
				  case STB_GLOBAL: XMLAddString(ostr, 2, "binding", "GLOBAL"); break;
				  case STB_WEAK: XMLAddString(ostr, 2, "binding", "WEAK"); break;
				  case STB_NUM: XMLAddString(ostr, 2, "binding", "NUM"); break;
				  case STB_LOOS: XMLAddString(ostr, 2, "binding", "LOOS"); break;
				  case STB_HIOS: XMLAddString(ostr, 2, "binding", "HIOS"); break;
				  case STB_LOPROC: XMLAddString(ostr, 2, "binding", "LOPROC"); break;
				  case STB_HIPROC: XMLAddString(ostr, 2, "binding", "HIPROC"); break;
				  default: XMLAddString(ostr, 2, "binding", "UNKNOWN"); break;
			  }
        XMLAddTagEnd(ostr, 1, "var");
      }
	  }
  }
  XMLAddTagEnd(ostr, 0, "vars");
  map->SetVariables(ostr.str().data());

  printf("Loaded in theory.\n");
  NaClLoadDwarf(map);
  return map;

failed:
  if (map)
    map->Free();
  return NULL;
}

} /* End of NaCl Namespace */
