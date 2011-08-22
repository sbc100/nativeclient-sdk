// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <gcroot.h>

#include "dwarf_reader/dwarf_vm.h"
#include "elf_reader/elf_object.h"

#include "src/NaClVsx.DebugHelpers/DwarfParser.h"


using System::IntPtr;
using System::Runtime::InteropServices::GCHandle;
using System::Runtime::InteropServices::GCHandleType;
using System::Runtime::InteropServices::Marshal;

namespace NaClVsx {

class DwarfParserImpl : public dwarf_reader::IDwarfReader {
  public:

    explicit DwarfParserImpl(gcroot<NaClVsx::IDwarfReader^> target)
      : target_(target) {}

    //
    // Implementation of IDwarfReader
    //
    virtual void *StartCompilationUnit(
        uint64 offset,
        uint8 address_size,
        uint8 offset_size,
        uint64 cu_length,
        uint8 dwarf_version) {
      target_->StartCompilationUnit();
      return NULL;
    }

    virtual void EndCompilationUnit(void *ctx, uint64 offset) {
      target_->EndCompilationUnit();
    }

    virtual void *StartDIE(void *ctx,
        uint64 parent,
        uint64 offset,
        dwarf2reader::DwarfTag tag) {
      target_->StartDIE(parent, offset, static_cast<DwarfTag>(tag));

      return NULL;
    }

    virtual void EndDIE(void *ctx, uint64 offset) {
      target_->EndDIE(offset);
    }

    virtual void ProcessAttributeUnsigned(void *ctx,
        uint64 offset,
        uint64 parent,
        dwarf2reader::DwarfAttribute attr,
        dwarf2reader::DwarfForm form,
        uint64 data) {
      target_->ProcessAttribute(
        offset,
        parent,
        static_cast<DwarfAttribute>(attr),
        data);
    }

    virtual void ProcessAttributeSigned(void *ctx,
        uint64 offset,
        uint64 parent,
        dwarf2reader::DwarfAttribute attr,
        dwarf2reader::DwarfForm form,
        int64 data) {
      target_->ProcessAttribute(
        offset,
        parent,
        static_cast<DwarfAttribute>(attr),
        data);
    }

    virtual void ProcessAttributeReference(void *ctx,
        uint64 offset,
        uint64 parent,
        dwarf2reader::DwarfAttribute attr,
        dwarf2reader::DwarfForm form,
        uint64 data) {
      target_->ProcessAttribute(
        offset,
        parent,
        static_cast<DwarfAttribute>(attr),
        gcnew DwarfReference(data));
    }

    virtual void ProcessAttributeBuffer(void *ctx,
        uint64 offset,
        uint64 parent,
        dwarf2reader::DwarfAttribute attr,
        dwarf2reader::DwarfForm form,
        const char* data,
        uint64 len) {
      array<System::Byte>^ arr =
        gcnew array<System::Byte>(static_cast<int>(len));
      // NOTE: using const_cast because Marshal doesn't support const ptrs.
      Marshal::Copy(
        IntPtr(const_cast<char*>(data)), arr, 0, static_cast<int>(len));

      target_->ProcessAttribute(
        offset,
        parent,
        static_cast<DwarfAttribute>(attr),
        arr);
    }

    virtual void ProcessAttributeString(void *ctx,
        uint64 offset,
        uint64 parent,
        dwarf2reader::DwarfAttribute attr,
        dwarf2reader::DwarfForm form,
        const char* data) {
      // NOTE: using const_cast because Marshal doesn't support const ptrs.
      System::String^ str =
        Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(data)));

      target_->ProcessAttribute(
        offset,
        parent,
        static_cast<DwarfAttribute>(attr),
        str);
    }

    virtual void DefineDir(void *ctx, const char *name, uint32 dir_num) {
      // NOTE: using const_cast because Marshal doesn't support const ptrs.
      target_->DefineDir(
        Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(name))),
        dir_num);
    }

    virtual void DefineFile(void *ctx, const char *name, int32 file_num,
        uint32 dir_num, uint64 mod_time,
        uint64 length) {
      // NOTE: using const_cast because Marshal doesn't support const ptrs.
      target_->DefineFile(
        Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(name))),
        file_num,
        dir_num);
    }

    virtual void AddLine(void *ctx,
        uint64 address,
        uint64 length,
        uint32 file_num,
        uint32 line_num,
        uint32 column_num) {
      target_->AddLine(address, length, file_num, line_num, column_num);
    }

    virtual void AddLocListEntry(uint64 offset,
        bool is_first_entry,
        uint64 lowPc,
        uint64 highPc,
        const void* data,
        size_t dataSize) {
      array<System::Byte>^ arr =
        gcnew array<System::Byte>(static_cast<int>(dataSize));
      // NOTE: using const_cast because Marshal doesn't support const ptrs.
      Marshal::Copy(
        IntPtr(const_cast<void*>(data)), arr, 0, static_cast<int>(dataSize));

      target_->AddLocListEntry(offset, is_first_entry, lowPc, highPc, arr);
    }

    virtual bool BeginCfiEntry(
        size_t offset,
        uint64 address,
        uint64 length,
        uint8 version,
        const char* augmentation,
        unsigned return_address) {
      return target_->BeginCfiEntry(
        address);
    }

    virtual bool AddCfiRule(
        uint64 address,
        int reg,
        CFI_RuleType ruleType,
        int base_register,
        int32 offset,
        const void* data,
        uint32_t len) {
      // NOTE: using const_cast because Marshal doesn't support const ptrs.
      void* void_data = const_cast<void*>(data);
      array<System::Byte>^ arr = nullptr;
      if (data && len) {
        gcnew array<System::Byte>(static_cast<int>(len));
        Marshal::Copy(IntPtr(void_data), arr, 0, static_cast<int>(len));
      }

      return target_->AddCfiRule(
          address,
          reg,
          (NaClVsx::IDwarfReader::CfiRuleType)ruleType,
          base_register,
          offset,
          arr);
    }

    virtual bool EndCfiEntry() {
      return target_->EndCfiEntry();
    }

    virtual void AddRangeListEntry(uint64 offset,
                                   uint64 base_address,
                                   uint64 low_pc,
                                   uint64 high_pc) {
      target_->AddRangeListEntry(offset, base_address, low_pc, high_pc);
    }

  private:
    gcroot<NaClVsx::IDwarfReader^> target_;
};

  class DwarfVmImpl : public dwarf_reader::IDwarfVM {
  public:
    explicit DwarfVmImpl(gcroot<NaClVsx::IDwarfVM^> vm) : target_(vm) {}

    virtual uint32_t BitWidth() {
      return target_->BitWidth();
    }

    virtual bool IsLSB() {
      return target_->IsLSB();
    }

    virtual void ErrorString(const char *str) {
      target_->ErrorString(
        Marshal::PtrToStringAnsi(IntPtr(const_cast<char*>(str))));
    }

    virtual uint64_t ReadRegister(int reg_number) {
      return target_->ReadRegister(reg_number);
    }

    virtual uint64_t ReadMemory(uint64_t address, int count) {
      return target_->ReadMemory(address, count);
    }

    virtual uint64_t ReadFrameBase() {
      return target_->ReadFrameBase();
    }

  private:
    gcroot<NaClVsx::IDwarfVM^> target_;
  };




  void DwarfParser::DwarfParseElf(
      System::String^ filename,
      IDwarfReader^ reader ) {
    elf_reader::ElfObject elf;
    DwarfParserImpl parse = DwarfParserImpl(gcroot<IDwarfReader^>(reader));

    IntPtr hString = Marshal::StringToHGlobalAnsi(filename);
    bool result = elf.Load(reinterpret_cast<char*>(hString.ToPointer()));
    Marshal::FreeHGlobal(hString);

    if (result) {
      dwarf_reader::DwarfParseElf(&elf, &parse);
    } else {
      throw gcnew System::IO::FileLoadException("Could not load " + filename);
    }
  }

  uint64_t DwarfParser::DwarfParseVM(
      IDwarfVM^ vm,
      array<System::Byte>^ data) {
    DwarfVmImpl vmImpl = DwarfVmImpl(gcroot<IDwarfVM^>(vm));
    GCHandle^ hData = GCHandle::Alloc(data, GCHandleType::Pinned);
    IntPtr pData = hData->AddrOfPinnedObject();

    uint64_t result = dwarf_reader::DwarfParseVM(
      &vmImpl,
      reinterpret_cast<uint8_t*>(pData.ToPointer()),
      data->Length);

    hData->Free();
    return result;
  }
}  // namespace NaClVsx

