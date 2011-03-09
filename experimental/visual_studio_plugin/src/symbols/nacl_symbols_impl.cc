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
#include "nacl_xml_helpers.h"

#include "common\dwarf_cu_to_module.h"
#include "common\dwarf_line_to_module.h"


using namespace std;

extern "C" char *elf_getedata(Elf *elf);

namespace NaCl {

extern bool NaClLoadDwarf(SymbolMapImpl *impl);

static int searchAddr(const void *key, const void *sym) {
  MapModule::Function *func = *((MapModule::Function **) sym);
  uint32_t addr = (uint32_t) key;

  if (addr < func->address)
    return -1;
  if (addr >= (func->address + func->size))
    return 1;
  
  return 0;  
}


SymbolMapImpl::SymbolMapImpl(int fd, Elf *elf, const char *file) 
: fd_(fd),
  elf_(elf) {
  
  module_ = new google_breakpad::Module(file, "Windows", "x86_64", "id");
  file_context_ = new google_breakpad::DwarfCUToModule::FileContext(file, module_);

}

SymbolMapImpl::~SymbolMapImpl() {
  if (elf_)
    elf_end(elf_);

  if (fd_ > 0)
    _close(fd_);
}

void SymbolMapImpl::Free() {
  delete this;
}

Elf *SymbolMapImpl::GetElf() { return elf_; }
MapFileContext *SymbolMapImpl::GetFileContext() { return file_context_; }
MapModule *SymbolMapImpl::GetModule() { return module_; }

static void AppendFunction(MapModule::Function* pfunc, stringstream& ostr, uint32_t lvl) {
  uint32_t lvl2 = 0, lvl3 = 0, lvl4 = 0;
  if (lvl) {
    lvl2 = lvl + 1;
    lvl3 = lvl + 2;
    lvl4 = lvl + 3;
  }

  XMLAddTagStart(ostr, lvl, "function");
    XMLAddString(ostr, lvl2, "name", pfunc->name);
    XMLAddPointer(ostr, lvl2, "address", pfunc->address);
    XMLAddNumber(ostr, lvl2, "size", pfunc->size);
    XMLAddNumber(ostr, lvl2, "parameter_size", pfunc->parameter_size);
    XMLAddTagStart(ostr, lvl2, "locations");
    for (uint32_t a=0; a < pfunc->lines.size(); a++) {
      XMLAddTagStart(ostr, lvl3, "location");
        XMLAddNumber(ostr, lvl4, "source-id",pfunc->lines[a].file->source_id);
        XMLAddNumber(ostr, lvl4, "line",pfunc->lines[a].number);
        XMLAddPointer(ostr, lvl4, "address",pfunc->lines[a].address);
        XMLAddNumber(ostr, lvl4, "size", pfunc->lines[a].size);
      XMLAddTagEnd(ostr, lvl3, "location");
    }
   XMLAddTagEnd(ostr, lvl2, "locations");
  XMLAddTagEnd(ostr, lvl, "function");
}

static void AppendSource(MapModule::File *pfile, stringstream& ostr, uint32_t lvl) {
  uint32_t lvl2 = 0;
  if (lvl)
    lvl2 = lvl + 1;

  XMLAddTagStart(ostr, lvl, "file");
  XMLAddString(ostr, lvl2, "path", pfile->name);
  XMLAddNumber(ostr, lvl2, "source-id", pfile->source_id);
  XMLAddTagEnd(ostr, lvl, "file");
}

static void AppendSection(const string& name, MapSection *psec, stringstream& ostr, uint32_t lvl) {
  uint32_t lvl2 = 0;
  if (lvl)
    lvl2 = lvl + 1;

  XMLAddTagStart(ostr, lvl, "section");
    XMLAddString(ostr, lvl2, "name", name);
    XMLAddPointer(ostr, lvl2, "data", (uint64_t) psec->first);
    XMLAddPointer(ostr, lvl2, "size", (uint64_t) psec->second);
  XMLAddTagEnd(ostr, lvl, "section");
}


static void AppendModule(MapModule *mod, stringstream& ostr, uint32_t lvl) {
  uint32_t lvl2 = 0, lvl3 = 0, lvl4 = 0;
  vector<MapModule::Function *> funcs;
  vector<MapModule::File *> files;

  if (lvl) {
    lvl2 = lvl + 1;
    lvl3 = lvl + 2;
    lvl4 = lvl + 3;
  }

  mod->GetFunctions(&funcs, funcs.begin());
  mod->GetFiles(&files);


    XMLAddTagStart(ostr, lvl2, "files");
    for (uint32_t fileIndex = 0; fileIndex < files.size(); fileIndex++)
      AppendSource(files[fileIndex], ostr, lvl3);
    XMLAddTagEnd(ostr, lvl2, "files");

    XMLAddTagStart(ostr, lvl2, "functions");
    for (uint32_t funcIndex = 0; funcIndex < funcs.size(); funcIndex++)
      AppendFunction(funcs[funcIndex], ostr, lvl3);
    XMLAddTagEnd(ostr, lvl2, "functions");
}


ISymbolMap::MapResult SymbolMapImpl::SourcesXML(AutoStr *xml) {
  vector<MapModule::File *> files;
  stringstream ostr;

  if (NULL == xml)
    return BAD_ARGS;

  if (NULL == module_) 
    return BAD_MAP;

  module_->GetFiles(&files);

  XMLAddTagStart(ostr, 0, "files");
  for (uint32_t fileIndex = 0; fileIndex < files.size(); fileIndex++)
    AppendSource(files[fileIndex], ostr, 1);
  XMLAddTagEnd(ostr, 0, "files");

  XMLAllocString(ostr,xml);
  return OK;
}

ISymbolMap::MapResult SymbolMapImpl::SectionsXML(AutoStr *xml) {
  stringstream ostr;
  dwarf2reader::SectionMap &secs = GetFileContext()->section_map;
  dwarf2reader::SectionMap::iterator iter;

  if (NULL == xml)
    return BAD_ARGS;

  if (NULL == module_) 
    return BAD_MAP;

  XMLAddTagStart(ostr, 0, "sections"); 
  for (iter = secs.begin(); iter != secs.end(); iter++) {
    XMLAddTagStart(ostr, 0, "section");
    AppendSection((*iter).first, &(*iter).second, ostr, 1);
    XMLAddTagEnd(ostr, 0, "sections");
  }
  XMLAllocString(ostr,xml);

  return OK;
}

ISymbolMap::MapResult SymbolMapImpl::LocationToAddrXML(uint32_t sourceId, uint32_t line, AutoStr *xml) {
  vector<MapModule::File *> files;
  vector<MapModule::Function *> funcs;
  stringstream ostr;

  MapModule::File *file;

  if (NULL == xml)
    return BAD_ARGS;

  if (NULL == module_) 
    return BAD_MAP;

  module_->GetFiles(&files);
  module_->GetFunctions(&funcs, funcs.begin());

  if (sourceId >= files.size()) {
    ostr << "<error>Source file index out of range.</error>";
    XMLAllocString(ostr,xml);
    return BAD_RANGE;
  }

  file = files[sourceId];

  XMLAddTagStart(ostr, 0, "addrs");
  for (uint32_t funcIndex = 0; funcIndex < files.size(); funcIndex++) {
    MapModule::Function *func = funcs[funcIndex];
    for (uint32_t lineIndex = 0; lineIndex < func->lines.size(); lineIndex++) {
      if ((func->lines[lineIndex].number == line) && (func->lines[lineIndex].file == file)) {
        XMLAddPointer(ostr, 0, "addr", func->lines[lineIndex].address);
      }
    }
  }
  XMLAddTagEnd(ostr, 0, "addrs");
  XMLAllocString(ostr,xml);

  return OK;
}


ISymbolMap::MapResult SymbolMapImpl::AddrToFunctionXML(uint32_t addr, AutoStr* xml) {
  google_breakpad::Module *mod = GetModule();
  google_breakpad::Module::Function **ppfunc;
  vector<google_breakpad::Module::Function *> funcs;
  
  mod->GetFunctions(&funcs, funcs.begin());
  ppfunc = (MapModule::Function **) bsearch((const void *) addr, 
                                  &funcs[0],
                                  funcs.size(),
                                  sizeof(MapModule::Function *),
                                  searchAddr);

  stringstream ostr;

  if (NULL == xml)
    return BAD_ARGS;

  if (NULL == module_) 
    return BAD_MAP;

  XMLAddTagStart(ostr, 0, "functions");
  if (ppfunc) {
    google_breakpad::Module::Function *pfunc = *ppfunc;
    AppendFunction(pfunc, ostr, 1);
  }
  XMLAddTagEnd(ostr, 0, "functions");
  XMLAllocString(ostr,xml);

  return OK;
}

ISymbolMap::MapResult SymbolMapImpl::ModuleXML(AutoStr *xml) {
  stringstream ostr;

  if (NULL == xml)
    return BAD_ARGS;

  if (NULL == module_) 
    return BAD_MAP;

  XMLAddTagStart(ostr, 0, "module");
  AppendModule(GetModule(), ostr, 1);

  // Append pre-computed variable info
  ostr << GetVariables();
  XMLAddTagEnd(ostr, 0, "module");

  XMLAllocString(ostr, xml);
  return OK;
}

uint64_t SymbolMapImpl::LocationToAddr(const char *name, uint32_t srcline) {
  vector<MapModule::Function *> funcs;
  MapModule::Function* func = NULL;
  uint32_t floop = 0, lloop = 0;

  module_->GetFunctions(&funcs, funcs.begin());

  for (floop = 0; floop < funcs.size(); floop++) {
    func = funcs[floop];
    for (lloop = 0; lloop < func->lines.size(); lloop++) {
      MapModule::Line *line = &func->lines[lloop];
      if ((line->file->name == name) && (line->number == srcline))
        return line->address;
    }
  }

  return 0;
}

const char *SymbolMapImpl::AddrToLocation(uint32_t addr, uint32_t *line) {
  google_breakpad::Module *mod = GetModule();
  google_breakpad::Module::Function **ppfunc;
  vector<google_breakpad::Module::Function *> funcs;
  
  mod->GetFunctions(&funcs, funcs.begin());
  ppfunc = (MapModule::Function **) bsearch((const void *) addr, 
                                  &funcs[0],
                                  funcs.size(),
                                  sizeof(MapModule::Function *),
                                  searchAddr);

  if (ppfunc) {
    google_breakpad::Module::Function *pfunc = *ppfunc;
    uint32_t loop;
    MapModule::File *file;

    for (loop=0; loop < pfunc->lines.size(); loop++) {
      if (pfunc->lines[loop].address == addr) {
        *line = pfunc->lines[loop].number;
        return pfunc->lines[loop].file->name.data();
      }
    }
  }

  *line = 0;
  return "UNKNOWN";
}


ISymbolMap::MapResult SymbolMapImpl::VariablesToXML(AutoStr *xml) {
  if (NULL == xml)
    return BAD_ARGS;

  if (NULL == module_) 
    return BAD_MAP;

  char* data = new char[variables_.length()+1];
  strcpy(data, variables_.data());
  xml->SetStr(data, XMLFreeString);
  return OK;
}


void SymbolMapImpl::SetVariables(const char *str) {
  variables_ = str;
}

const char *SymbolMapImpl::GetVariables() const {
  return variables_.data();
}


} /* End of NaCl Namespace */
