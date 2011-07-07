#include "rsp_registers.h"

// Ok, now we have a definite answer on how gdbserver package registers
// in 'g' command response.
// You have to download gdb source code, and look there:
// 
// <gdb>\gdb-6.3.50.20051116\gdb\regformats\reg-i32.dat
// <gdb>\gdb-6.3.50.20051116\gdb\regformats\reg-x86-64.dat
//
// name:i386
// expedite:ebp,esp,eip
// 32:eax
// 32:ecx
// 32:edx
// 32:ebx
// 32:esp
// 32:ebp
// 32:esi
// 32:edi
// 32:eip
// 32:eflags
// 32:cs
// 32:ss
// 32:ds
// 32:es
// 32:fs
// 32:gs
// 
// name:x86_64
// expedite:rbp,rsp,rip
// 64:rax
// 64:rbx
// 64:rcx
// 64:rdx
// 64:rsi
// 64:rdi
// 64:rbp
// 64:rsp
// 64:r8
// 64:r9
// 64:r10
// 64:r11
// 64:r12
// 64:r13
// 64:r14
// 64:r15
// 64:rip
// 32:eflags
// 32:cs
// 32:ss
// 32:ds
// 32:es
// 32:fs
// 32:gs

namespace {
#define X86_32_REGS \
  REG(Eax); \
  REG(Ecx); \
  REG(Edx); \
  REG(Ebx); \
  REG(Esp); \
  REG(Ebp); \
  REG(Esi); \
  REG(Edi); \
  REG(Eip); \
  REG(EFlags); \
  REG(SegCs); \
  REG(SegSs); \
  REG(SegDs); \
  REG(SegEs); \
  REG(SegFs); \
  REG(SegGs)

#define X86_64_REGS \
  REG(Rax); \
  REG(Rbx); \
  REG(Rcx); \
  REG(Rdx); \
  REG(Rsi); \
  REG(Rdi); \
  REG(Rbp); \
  REG(Rsp); \
  REG(R8); \
  REG(R9); \
  REG(R10); \
  REG(R11); \
  REG(R12); \
  REG(R13); \
  REG(R14); \
  REG(R15); \
  REG(Rip); \
  REG(EFlags); \
  REG(SegCs); \
  REG(SegSs); \
  REG(SegDs); \
  REG(SegEs); \
  REG(SegFs); \
  REG(SegGs)

// Registers SegCs,SegSs,SegDs,SegEs,SegFs,SegGs are defined as WORD (16-bit)
// in winnt.h for win-64, but passed as 32-bit by RSP protocol.
char* names_of_bad_regs = "SegCs,SegSs,SegDs,SegEs,SegFs,SegGs,";

// EFlags, ???

size_t CopyRegisterFromBlobToCONTEXT(const debug::Blob& blob, size_t offset, void* dst, int reg_size, const char* name) {
#ifdef _WIN64
  char* pp = strstr(names_of_bad_regs,name);
  size_t name_len = strlen(name);
  if ((NULL != pp) && (',' == *(pp + name_len))) {
    unsigned long reg = 0;
    reg_size = sizeof(reg);
    blob.Peek(offset, &reg, sizeof(reg));
    memcpy(dst, &reg, 2);
  } else {
    blob.Peek(offset, dst, reg_size);
  }
#else
    blob.Peek(offset, dst, reg_size);
#endif
  return offset + reg_size;
}

void CopyRegisterFromCONTEXTToBlob(const void* src, int reg_size, const char* name, debug::Blob* blob) {
  char* pp = strstr(names_of_bad_regs,name);
  size_t name_len = strlen(name);
  if ((NULL != pp) && (',' == *(pp + name_len))) {
    unsigned long reg = 0;
    memcpy(&reg, src, 2);
    blob->Append(debug::Blob(&reg, sizeof(reg)));
  } else {
    blob->Append(debug::Blob(src, reg_size));
  }
}
}  // namespace

namespace rsp {
void GdbRegistersToCONTEXT(const debug::Blob& gdb_regs, CONTEXT* ct) {
  WORD sz = 0;
  size_t offset = 0;
#define REG(name) ct->name = 0;\
  offset = CopyRegisterFromBlobToCONTEXT(gdb_regs,\
                                         offset,\
                                         &ct->name,\
                                         sizeof(ct->name),\
                                         #name)
#ifdef _WIN64
  X86_64_REGS;
#else
  X86_32_REGS;
#endif  
#undef REG
}

void CONTEXTToGdbRegisters(const CONTEXT& ct, debug::Blob* gdb_regs) {
#define REG(name) CopyRegisterFromCONTEXTToBlob(&ct.name, sizeof(ct.name), #name, gdb_regs)
#ifdef _WIN64
  X86_64_REGS;
#else
  X86_32_REGS;
#endif  
#undef REG
}

#undef X86_32_REGS
#undef X86_64_REGS
}  // namespace rsp
