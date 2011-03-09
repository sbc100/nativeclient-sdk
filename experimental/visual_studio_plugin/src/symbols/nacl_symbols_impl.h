#ifndef NACL_SYMBOLS_IMPL_H_
#define NACL_SYMBOLS_IMPL_H_ 1

#include <map>
#include <vector>
#include <string>
#include <strstream>

#include "libelf.h"
#include "native_client/src/include/portability.h"
#include "common\module.h"
#include "common\dwarf\dwarf2reader.h"
#include "common\dwarf_cu_to_module.h"
#include "common\dwarf_line_to_module.h"

#include "nacl_symbols.h"

typedef google_breakpad::Module MapModule;
typedef google_breakpad::DwarfCUToModule::FileContext MapFileContext;
typedef pair<const char*, uint64> MapSection;

namespace NaCl {

class SymbolMapImpl : public ISymbolMap {
public:
  SymbolMapImpl(int fd, Elf *elf, const char *file);

public:
  void Free();

public:
  MapResult ModuleXML(AutoStr *out_xml);
  MapResult AddrToFunctionXML(uint32_t addr, AutoStr *xml);
  MapResult LocationToAddrXML(uint32_t sourceId, uint32_t line, AutoStr *xml);
  MapResult SourcesXML(AutoStr *xml);
  MapResult SectionsXML(AutoStr *xml);
  MapResult VariablesToXML(AutoStr *out_xml);

public:
  uint64_t LocationToAddr(const char *name, uint32_t line);
  const char *AddrToLocation(uint32_t addr, uint32_t *line);

public:
  Elf *GetElf();
  MapModule *GetModule();
  MapFileContext *GetFileContext();
  
  void SetVariables(const char *str);
  const char *GetVariables() const;

protected:
  virtual ~SymbolMapImpl();

private:
  int fd_;
  Elf *elf_;
  std::string variables_;

//  std::vector<SymbolSection *> sections;
  MapModule *module_;
  MapFileContext *file_context_;
};

} /* End of NaCl namespace */

#endif