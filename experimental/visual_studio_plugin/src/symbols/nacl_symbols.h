#ifndef NACL_SYMBOLS_H_
#define NACL_SYMBOLS_H_ 1

#include <sstream>

#include "native_client/src/include/portability.h"

namespace NaCl {

class AutoStr {
public:
  typedef void (*CleanupFunc)(char* p);
  
  AutoStr():str_(NULL),cleanup_(NULL){}
  ~AutoStr() {
    SetStr(NULL, NULL);
  }

  void SetStr(char* str, CleanupFunc cleanup) {
    if (str_) {
      if (cleanup_) {
        cleanup_(str_);
      }
      str_ = NULL;
    }
    str_ = str;
    cleanup_ = cleanup;
  }

  const char* GetStr() {
    return str_;
  }
private:
  // nocopy
  AutoStr(const AutoStr&);
  AutoStr& operator = (const AutoStr&);

  char* str_;
  CleanupFunc cleanup_;
};

class ISymbolMap {
protected:
  virtual ~ISymbolMap(){}

public:
  enum MapResult {
    FAILED = -4,
    BAD_MAP = -3,
    BAD_RANGE = -2,
    BAD_ARGS = -1,
    NOT_READY = 0,
    OK = 1,
  };

public:
  virtual void Free() = 0;

public:
  virtual MapResult ModuleXML(AutoStr *out_xml) = 0;
  virtual MapResult AddrToFunctionXML(uint32_t addr, AutoStr *out_xml) = 0;
  virtual MapResult LocationToAddrXML(uint32_t sourceId, uint32_t line, AutoStr *out_xml) = 0;
  virtual MapResult SourcesXML(AutoStr *out_xml) = 0;
  virtual MapResult SectionsXML(AutoStr *out_xml)= 0;
  virtual MapResult VariablesToXML(AutoStr *out_xml) = 0;

  virtual uint64_t LocationToAddr(const char *name, uint32_t line) = 0;
  virtual const char *AddrToLocation(uint32_t addr, uint32_t *line) = 0;
};

ISymbolMap *SymbolMapFactory(const char *str);

} /* End NaCl Namespace */



#endif  /* NACL_SYMBOLS_H_ */

