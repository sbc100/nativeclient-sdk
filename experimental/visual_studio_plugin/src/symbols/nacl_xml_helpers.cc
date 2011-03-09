#include <string>
#include <sstream>

#include "nacl_xml_helpers.h"

using namespace std;

namespace NaCl {

void XMLFreeString(char* str) {
  delete[] str;
}

void XMLAllocString(const stringstream& ostr, AutoStr* str) {
  char* data = new char[ostr.str().length()+1];
  strcpy(data, ostr.str().data());
  str->SetStr(data, XMLFreeString);
}

static void XMLAddIndent(stringstream &sstr, uint32_t lvl) {
  for (uint32_t loop = 0; loop < lvl; loop++)
    sstr << '\t';
}

void XMLAddTagStart(std::stringstream &sstr, uint32_t lvl, const std::string &tag) {
  XMLAddIndent(sstr,lvl);
  sstr << "<";
  sstr << tag;
  sstr << ">\n";
}

void XMLAddTagEnd(std::stringstream &sstr, uint32_t lvl, const std::string &tag) {
  XMLAddIndent(sstr,lvl);
  sstr << "</";
  sstr << tag;
  sstr << ">\n";
}

void XMLAddString(stringstream &sstr, uint32_t lvl, const string &tag, const string &str) {
  uint32_t loop;

  XMLAddIndent(sstr,lvl);
  sstr << "<";
  sstr << tag;
  sstr << ">";

  const char *in = str.data();
  for (loop =0; in[loop]; loop++) {
    char ch = in[loop];
    if ('&' == ch) {
      sstr << "&amp;"; 
      continue;
    }
    if ('<' == ch) {
      sstr << "&lt;";
      continue;
    }
    if ('>' == ch) { 
      sstr << "&gt;";
      continue;
    }
    if ('"' == ch) {
      sstr << "&quot;";
      continue;
    }
    if ('\'' == ch) {
      sstr << "&apos;";
      continue;
    }
   if (ch < ' ') {
      uint32_t val = (uint32_t) ch;
      sstr << "&#";
      sstr << val;
      sstr << ";";
      continue;
    }

    sstr << ch;
  }

  sstr << "</";
  sstr << tag;
  sstr << ">\n";
}

void XMLAddNumber(stringstream &sstr, uint32_t lvl, const string &tag, uint64_t number) {
  XMLAddIndent(sstr,lvl);
  sstr << "<";
  sstr << tag;
  sstr << ">";
  sstr.setf(ios::dec, ios::basefield);
  sstr << number;
  sstr << "</";
  sstr << tag;
  sstr << ">\n";
}

void XMLAddPointer(stringstream &sstr, uint32_t lvl, const string &tag, uint64_t number) {
  XMLAddIndent(sstr,lvl);
  sstr << "<";
  sstr << tag;
  sstr << ">0x";
  sstr.setf(ios::hex, ios::basefield);
  sstr.setf(0, ios::showbase);
  sstr << number;
  sstr << "</";
  sstr << tag;
  sstr << ">\n";
}

} /* End of NaCl Namespace */