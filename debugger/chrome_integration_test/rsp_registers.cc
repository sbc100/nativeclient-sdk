// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/chrome_integration_test/rsp_registers.h"
#include "debugger/base/debug_blob.h"
#include "debugger/chrome_integration_test/windows_thread_context.h"

namespace debug {
#define REG(sz, name) uint##sz##_t name
#pragma pack(push, 1)
struct GdbRegisters64 {
  REG(64, rax);
  REG(64, rbx);
  REG(64, rcx);
  REG(64, rdx);
  REG(64, rsi);
  REG(64, rdi);
  REG(64, bp);
  REG(64, sp);
  REG(64, r8);
  REG(64, r9);
  REG(64, r10);
  REG(64, r11);
  REG(64, r12);
  REG(64, r13);
  REG(64, r14);
  REG(64, r15);
  REG(64, ip);
  REG(32, eflags);
  REG(32, cs);
  REG(32, ss);
  REG(32, ds);
  REG(32, es);
  REG(32, fs);
  REG(32, gs);
};

struct GdbRegisters32 {
  REG(32, eax);
  REG(32, ecx);
  REG(32, edx);
  REG(32, ebx);
  REG(32, sp);
  REG(32, bp);
  REG(32, esi);
  REG(32, edi);
  REG(32, ip);
  REG(32, eflags);
  REG(32, cs);
  REG(32, ss);
  REG(32, ds);
  REG(32, es);
  REG(32, fs);
  REG(32, gs);
};
#pragma pack(pop)

void RegistersSet::AddReg(int gdb_number,
                          std::string gdb_name,
                          int gdb_offset,
                          int gdb_size,
                          int context_offset,
                          int context_size) {
  RegisterDescription desc;
  desc.gdb_number_ = gdb_number;
  desc.gdb_name_ = gdb_name;
  desc.gdb_offset_ = gdb_offset;
  desc.gdb_size_ = gdb_size;
  desc.context_offset_ = context_offset;
  desc.context_size_ = context_size;
  regs_.push_back(desc);
}

void RegistersSet::InitializeForWin64() {
#define MAP_REG(gdb_name, win_name) \
  AddReg(number++, \
         #gdb_name, \
         offsetof(GdbRegisters64, gdb_name), \
         sizeof(gdb_regs.##gdb_name), \
         offsetof(win64::CONTEXT, win_name), \
         sizeof(ct.##win_name))

  win64::CONTEXT ct;
  GdbRegisters64 gdb_regs;
  int number = 1;

  MAP_REG(rax, Rax);
  MAP_REG(rbx, Rbx);
  MAP_REG(rcx, Rcx);
  MAP_REG(rdx, Rdx);
  MAP_REG(rsi, Rsi);
  MAP_REG(rdi, Rdi);
  MAP_REG(bp, Rbp);
  MAP_REG(sp, Rsp);
  MAP_REG(r8, R8);
  MAP_REG(r9, R9);
  MAP_REG(r10, R10);
  MAP_REG(r11, R11);
  MAP_REG(r12, R12);
  MAP_REG(r13, R13);
  MAP_REG(r14, R14);
  MAP_REG(r15, R15);
  MAP_REG(ip, Rip);
  MAP_REG(eflags, EFlags);
  MAP_REG(cs, SegCs);
  MAP_REG(ss, SegSs);
  MAP_REG(ds, SegDs);
  MAP_REG(es, SegEs);
  MAP_REG(fs, SegFs);
  MAP_REG(gs, SegGs);
#undef MAP_REG
}

void RegistersSet::InitializeForWin32() {
#define MAP_REG(gdb_name, win_name) \
  AddReg(number++, \
         #gdb_name, \
         offsetof(GdbRegisters32, gdb_name), \
         sizeof(gdb_regs.##gdb_name), \
         offsetof(win32::CONTEXT, win_name), \
         sizeof(ct.##win_name))

  win32::CONTEXT ct;
  GdbRegisters32 gdb_regs;
  int number = 1;

  MAP_REG(eax, Eax);
  MAP_REG(ecx, Ecx);
  MAP_REG(edx, Edx);
  MAP_REG(ebx, Ebx);
  MAP_REG(sp, Esp);
  MAP_REG(bp, Ebp);
  MAP_REG(esi, Esi);
  MAP_REG(edi, Edi);
  MAP_REG(ip, Eip);
  MAP_REG(eflags, EFlags);
  MAP_REG(cs, SegCs);
  MAP_REG(ss, SegSs);
  MAP_REG(ds, SegDs);
  MAP_REG(es, SegEs);
  MAP_REG(fs, SegFs);
  MAP_REG(gs, SegGs);
#undef MAP_REG
}

void RegistersSet::PrintRegisters(const debug::Blob& blob) {
  for (size_t i = 0; i < regs_.size(); i++) {
    uint64_t reg_value = 0;
    std::string reg_name = regs_[i].gdb_name_;
    if (ReadRegisterFromGdbBlob(blob, reg_name, &reg_value))
      printf("%s = 0x%I64x\n", reg_name.c_str(), reg_value);
  }
}

bool RegistersSet::ReadRegisterFromGdbBlob(
    const debug::Blob& blob,
    const RegisterDescription& register_info,
    uint64_t* destination) const {
  return (register_info.gdb_size_ == blob.Peek(register_info.gdb_offset_,
                                               destination,
                                               register_info.gdb_size_));
}

const RegisterDescription* RegistersSet::FindRegisterDescription(
    const std::string& register_name) const {
  for (size_t i = 0; i < regs_.size(); i++)
    if (register_name == regs_[i].gdb_name_)
      return &regs_[i];
  return NULL;
}

bool RegistersSet::ReadRegisterFromGdbBlob(const debug::Blob& blob,
                                           const std::string& register_name,
                                           uint64_t* destination) const {
  const RegisterDescription* register_info =
      FindRegisterDescription(register_name);
  if (NULL == register_info)
    return false;
  return ReadRegisterFromGdbBlob(blob, *register_info, destination);
}

bool RegistersSet::WriteRegisterToGdbBlob(const std::string& register_name,
                                          uint64_t value,
                                          debug::Blob* destination_blob) const {
  const RegisterDescription* register_info =
      FindRegisterDescription(register_name);
  if (NULL == register_info)
    return false;
  WriteRegisterToGdbBlob(*register_info, value, destination_blob);
  return true;
}

void RegistersSet::WriteRegisterToGdbBlob(
    const RegisterDescription& register_info,
    uint64_t value,
    debug::Blob* destination_blob) const {
  size_t offs = register_info.gdb_offset_;
  size_t size = register_info.gdb_size_;
  while ((offs + size) > destination_blob->size())
    destination_blob->PushBack(0);

  uint8_t* src = reinterpret_cast<uint8_t*>(&value);
  for (size_t i = 0; i < size; i++) {
    (*destination_blob)[offs + i] = *src++;
  }
}

}  // namespace debug

