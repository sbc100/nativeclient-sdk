#ifndef NACL_XML_HELPERS_H_
#define NACL_XML_HELPERS_H_ 1

#include <sstream>

#include "native_client/src/include/portability.h"

#include "nacl_symbols.h"

namespace NaCl {

void XMLAddTagStart(std::stringstream &sstr, uint32_t lvl, const std::string &tag);
void XMLAddTagEnd(std::stringstream &sstr, uint32_t lvl, const std::string &tag);
void XMLAddString(std::stringstream &sstr, uint32_t lvl, const std::string &tag, const std::string &str);
void XMLAddNumber(std::stringstream &sstr, uint32_t lvl, const std::string &tag, uint64_t number);
void XMLAddPointer(std::stringstream &sstr, uint32_t lvl, const std::string &tag, uint64_t number);

void XMLFreeString(char* str);
void XMLAllocString(const std::stringstream& ostr, AutoStr* str);

} /* End of NaCl Namespace */

#endif  /* NACL_XML_HELPERS_H_ */