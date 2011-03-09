#include <stdio.h>
#include <stdlib.h>

#include "debug_symbols.h"

int main(int argc, const char *argv[])
{
  char *name = "D:\\private_svn\\nativeclient-vsx\\third_party\\dwarftest\\hello_world64.nexe";
  
  DSMapper_t *map = DebugSymbolsLoadNexe(name);  

  DSSymbol_t *sym;
  
  sym = map->AddrToFunc_(map, 0x20340);
  sym = map->NameToSym_(map, "main");
  
  map->Free_(map);
}
  

#if 0
  /* Iterate through section headers */
  while((scn = elf_nextscn(elf, scn)) != 0)
  {
	  // point shdr at this section header entry
	  gelf_getshdr(scn, &shdr);

		  // print the section header type
                  printf("\t %08x %08d \t", shdr.sh_offset, shdr.sh_size);
                  switch(shdr.sh_type)
                  {
                          case SHT_NULL: printf( "SHT_NULL\t");               break;
                          case SHT_PROGBITS: printf( "SHT_PROGBITS");       break;
                          case SHT_SYMTAB: printf( "SHT_SYMTAB");           break;
                          case SHT_STRTAB: printf( "SHT_STRTAB");           break;
                          case SHT_RELA: printf( "SHT_RELA\t");               break;
                          case SHT_HASH: printf( "SHT_HASH\t");               break;
                          case SHT_DYNAMIC: printf( "SHT_DYNAMIC");         break;
                          case SHT_NOTE: printf( "SHT_NOTE\t");               break;
                          case SHT_NOBITS: printf( "SHT_NOBITS");           break;
                          case SHT_REL: printf( "SHT_REL\t");                 break;
                          case SHT_SHLIB: printf( "SHT_SHLIB");             break;
                          case SHT_DYNSYM: printf( "SHT_DYNSYM");           break;
                          case SHT_INIT_ARRAY: printf( "SHT_INIT_ARRAY");   break;
                          case SHT_FINI_ARRAY: printf( "SHT_FINI_ARRAY");   break;
                          case SHT_PREINIT_ARRAY: printf( "SHT_PREINIT_ARRAY"); break;
                          case SHT_GROUP: printf( "SHT_GROUP");             break;
                          case SHT_SYMTAB_SHNDX: printf( "SHT_SYMTAB_SHNDX"); break;
                          case SHT_NUM: printf( "SHT_NUM\t");                 break;
                          case SHT_LOOS: printf( "SHT_LOOS\t");               break;
                          case SHT_GNU_verdef: printf( "SHT_GNU_verdef");   break;
                          case SHT_GNU_verneed: printf( "SHT_VERNEED");     break;
                          case SHT_GNU_versym: printf( "SHT_GNU_versym");   break;
                          default: printf( "(none) ");                      break;
                  }

		  // print the section header flags
		  printf("\t(");

                  if(shdr.sh_flags & SHF_WRITE) { printf("W"); }
                  if(shdr.sh_flags & SHF_ALLOC) { printf("A"); }
                  if(shdr.sh_flags & SHF_EXECmapR) { printf("X"); }
                  if(shdr.sh_flags & SHF_STRINGS) { printf("S"); }
		  printf(")\t");

	  // the shdr name is in a string table, libelf uses elf_strptr() to find it
	  // using the e_shstrndx value from the elf_header
	  printf("%s\n", elf_strptr(elf, elf_header->e_shstrndx, shdr.sh_name));
  }

  // Iterate through section headers again this time well stop when we find symbols 
  elf = elf_begin(fd, ELF_C_READ, NULL);


  while((scn = elf_nextscn(elf, scn)) != NULL)
  {
          gelf_getshdr(scn, &shdr);

	  // When we find a section header marked SHT_SYMTAB stop and get symbols
	  if(shdr.sh_type == SHT_SYMTAB)
          {
		  // edata points to our symbol table
		  edata = elf_getdata(scn, edata);

		  // how many symbols are there? this number comes from the size of
		  // the section divided by the entry size
		  symbol_count = shdr.sh_size / shdr.sh_entsize;

		  // loop through to grab all symbols
		  for(i = 0; i < symbol_count; i++)
                  {			
			  // libelf grabs the symbol data using gelf_getsym()
                          gelf_getsym(edata, i, &sym);

			  // print out the value and size
			  printf("%08x %08d ", sym.st_value, sym.st_size);
  	
			  // type of symbol binding
			  switch(ELF32_ST_BIND(sym.st_info))
			  {
				  case STB_LOCAL: printf("LOCAL"); break;
				  case STB_GLOBAL: printf("GLOBAL"); break;
				  case STB_WEAK: printf("WEAK"); break;
				  case STB_NUM: printf("NUM"); break;
				  case STB_LOOS: printf("LOOS"); break;
				  case STB_HIOS: printf("HIOS"); break;
				  case STB_LOPROC: printf("LOPROC"); break;
				  case STB_HIPROC: printf("HIPROC"); break;
				  default: printf("UNKNOWN"); break;
			  }

			  printf("\t");

			  // type of symbol
			  switch(ELF32_ST_TYPE(sym.st_info))
			  {
				  case STT_NOTYPE: printf("NOTYPE"); break;
				  case STT_OBJECT: printf("OBJECT"); break;
				  case STT_FUNC:  printf("FUNC"); break;
				  case STT_SECTION: printf("SECTION"); break;
				  case STT_FILE: printf("FILE"); break;
				  case STT_COMMON: printf("COMMON"); break;
				  case STT_TLS: printf("TLS"); break;
				  case STT_NUM: printf("NUM"); break;
				  case STT_LOOS: printf("LOOS"); break;
				  case STT_HIOS: printf("HIOS"); break;
				  case STT_LOPROC: printf("LOPROC"); break;
				  case STT_HIPROC: printf("HIPROC"); break;
				  default: printf("UNKNOWN"); break;
			  }

			  printf("\t");

			  // the name of the symbol is somewhere in a string table
			  // we know which one using the shdr.sh_link member
			  // libelf grabs the string using elf_strptr()
                          printf("%s\n", elf_strptr(elf, shdr.sh_link, sym.st_name));
                  }

	  }
  }
  return 1;

}

#endif