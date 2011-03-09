// Copyright 2009 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "src/NaClVsx.DebugHelpers/GdbProxy.h"

#include <string>

#include "third_party/nacl/native_client/src/trusted/debug_stub/debug_host.h"

using System::String;
using System::Byte;
using System::Int32;
using System::UInt32;
using System::UInt64;
using System::IntPtr;
using System::Collections::Generic::Dictionary;
using System::Diagnostics::Debug;
using System::Runtime::InteropServices::Marshal;
using System::Runtime::InteropServices::GCHandle;
using System::Runtime::InteropServices::GCHandleType;
using System::Runtime::InteropServices::OutAttribute;

using nacl_debug_conn::DebugHost;
using std::string;

namespace NaClVsx { namespace DebugHelpers {

ref class ClosureMap {
  public:
    static int AddClosure(GdbProxy::AsyncResponse^ closure, bool remove) {
      int id = nextId_++;
      closureHandleMap_[id] = closure;
      closureRemovalMap_[id] = remove;
      return id;
    }

    static GdbProxy::AsyncResponse^ GetClosure(int id) {
      GdbProxy::AsyncResponse^ result = closureHandleMap_[id];
      if (closureRemovalMap_[id]) {
        closureHandleMap_[id] = nullptr;
      }
      return result;
    }
  private:
    static Dictionary<int, GdbProxy::AsyncResponse^> closureHandleMap_;
    static Dictionary<int, bool> closureRemovalMap_;
    static int nextId_ = 0;
};

struct GdbProxyImpl {
  DebugHost *pHost;

  static void __stdcall DHAsync(DebugHost::DHResult res, void *obj);
  static void __stdcall DHAsyncStr(DebugHost::DHResult res,
      void *obj,
      const char *str);
  static void __stdcall DHAsyncMem(DebugHost::DHResult res,
      void *obj,
      void* data,
      uint32_t len);
};

void __stdcall GdbProxyImpl::DHAsync(DebugHost::DHResult res, void *obj) {
  GdbProxy::AsyncResponse^ closure =
    ClosureMap::GetClosure(reinterpret_cast<int>(obj));
  if (closure != nullptr) {
    closure((GdbProxy::ResultCode) res, nullptr, nullptr);
  }
}

void __stdcall GdbProxyImpl::DHAsyncStr(
    DebugHost::DHResult res,
    void *obj, const char *str) {
  GdbProxy::AsyncResponse^ closure =
    ClosureMap::GetClosure(reinterpret_cast<int>(obj));
  closure((GdbProxy::ResultCode) res, gcnew String(str), nullptr);
}

void __stdcall GdbProxyImpl::DHAsyncMem(
    DebugHost::DHResult res,
    void *obj,
    void* data,
    uint32_t len) {
  GdbProxy::AsyncResponse^ closure =
    ClosureMap::GetClosure(reinterpret_cast<int>(obj));
  array<Byte>^ managedData = gcnew array<Byte>(len);
  Marshal::Copy(IntPtr(data), managedData, 0, len);
  closure((GdbProxy::ResultCode) res, nullptr, managedData);
}

GdbProxy::GdbProxy(void)
    : pimpl_(new GdbProxyImpl) {
}

GdbProxy::~GdbProxy() {
  delete pimpl_;
}

bool GdbProxy::CanConnect(String^ connectionString) {
  Debug::WriteLine(String::Format("TransportImpl::CanConnect({0})",
                                  connectionString));

  return true;
}

void GdbProxy::Open(String^ connectionString) {
  Debug::WriteLine(String::Format("Transport::Open({0})",
                                  connectionString));

  IntPtr hString = Marshal::StringToHGlobalAnsi(connectionString);
  pimpl_->pHost = DebugHost::SocketConnect(
    static_cast<char*>(hString.ToPointer()));
  Marshal::FreeHGlobal(hString);

  if (!pimpl_->pHost) {
    throw gcnew System::IO::IOException(
      String::Format("Failed to connect to {0}", connectionString));
  }

  connectionString_ = connectionString;
}

void GdbProxy::Close() {
  Debug::WriteLine(String::Format("Transport::Close"));


  delete pimpl_->pHost;
  pimpl_->pHost = NULL;
}

bool GdbProxy::IsRunning() {
  return pimpl_->pHost->IsRunning();
}

void GdbProxy::SetOutputAsync(AsyncResponse^ reply) {
  pimpl_->pHost->SetOutputAsync(
    &GdbProxyImpl::DHAsyncStr,
    reinterpret_cast<void*>(ClosureMap::AddClosure(reply, false)));
}

void GdbProxy::SetStopAsync(AsyncResponse^ reply) {
  pimpl_->pHost->SetStopAsync(
    &GdbProxyImpl::DHAsync,
    reinterpret_cast<void*>(ClosureMap::AddClosure(reply, false)));
}



GdbProxy::ResultCode GdbProxy::GetPath(AsyncResponse^ reply) {
  return static_cast<ResultCode>(pimpl_->pHost->GetPathAsync(
      &GdbProxyImpl::DHAsyncStr,
      reinterpret_cast<void*>(ClosureMap::AddClosure(reply, true))));
}

GdbProxy::ResultCode GdbProxy::GetArch(AsyncResponse^ reply) {
  return static_cast<ResultCode>(pimpl_->pHost->GetArchAsync(
    &GdbProxyImpl::DHAsyncStr,
    reinterpret_cast<void*>(ClosureMap::AddClosure(reply, true))));
}

GdbProxy::ResultCode GdbProxy::GetThreads(AsyncResponse^ reply) {
  return static_cast<ResultCode>(pimpl_->pHost->GetThreadsAsync(
    &GdbProxyImpl::DHAsyncStr,
    reinterpret_cast<void*>(ClosureMap::AddClosure(reply, true))));
}

GdbProxy::ResultCode GdbProxy::GetLastSig([Out]int% sig) {
  int lastSig = 0;
  ResultCode result = ResultCode::DHR_FAILED;
  result = static_cast<ResultCode>(pimpl_->pHost->GetLastSig(&lastSig));
  sig = lastSig;
  return result;
}

GdbProxy::ResultCode GdbProxy::GetMemory(
    System::UInt64 offs,
    System::Object^ data) {
  Int32 size = Marshal::SizeOf(data->GetType());
  GCHandle^ hData = GCHandle::Alloc(data, GCHandleType::Pinned);
  IntPtr pData = hData->AddrOfPinnedObject();
  ResultCode result = static_cast<ResultCode>(
    pimpl_->pHost->GetMemory(offs, pData.ToPointer(), size));
  hData->Free();

  return result;
}

GdbProxy::ResultCode GdbProxy::GetMemory(
    System::UInt64 offs,
    System::Array^ data,
    System::UInt32 count ) {
  Int32 size =
      data->Length * Marshal::SizeOf(data->GetType()->GetElementType());
  Debug::Assert((UInt32)size >= count);
  GCHandle^ hData = GCHandle::Alloc(data, GCHandleType::Pinned);
  IntPtr pData = hData->AddrOfPinnedObject();
  ResultCode result = static_cast<ResultCode>(
    pimpl_->pHost->GetMemory(offs, pData.ToPointer(), count));
  hData->Free();

  return result;
}

GdbProxy::ResultCode GdbProxy::SetMemory(
    System::UInt64 offs,
    System::Array^ data,
    System::UInt32 count ) {
  Int32 size =
    data->Length * Marshal::SizeOf(data->GetType()->GetElementType());
  Debug::Assert((UInt32)size >= count);
  GCHandle^ hData = GCHandle::Alloc(data, GCHandleType::Pinned);
  IntPtr pData = hData->AddrOfPinnedObject();
  ResultCode result = static_cast<ResultCode>(
      pimpl_->pHost->SetMemory(offs, pData.ToPointer(), count));
  hData->Free();

  return result;
}

GdbProxy::ResultCode GdbProxy::GetRegisters(RegsX86_64^% registers) {
  Int32 size = Marshal::SizeOf(registers->GetType());
  GCHandle^ hRegisters = GCHandle::Alloc(registers, GCHandleType::Pinned);
  IntPtr pRegisters = hRegisters->AddrOfPinnedObject();
  ResultCode result = static_cast<ResultCode>(
      pimpl_->pHost->GetRegisters(pRegisters.ToPointer(), size));
  hRegisters->Free();

  return result;
}

GdbProxy::ResultCode GdbProxy::SetRegisters(void *, System::UInt32 ) {
  return ResultCode::DHR_FAILED;
}

GdbProxy::ResultCode GdbProxy::RequestBreak() {
  return static_cast<ResultCode>(pimpl_->pHost->RequestBreak());
}

GdbProxy::ResultCode GdbProxy::RequestContinueBackground() {
  return static_cast<ResultCode>(pimpl_->pHost->RequestContinueBackground());
}

GdbProxy::ResultCode GdbProxy::RequestContinue() {
  return static_cast<ResultCode>(pimpl_->pHost->RequestContinue());
}

GdbProxy::ResultCode GdbProxy::RequestStep() {
  return static_cast<ResultCode>(pimpl_->pHost->RequestStep());
}

bool GdbProxy::HasBreakpoint(System::UInt64 offs ) {
  return pimpl_->pHost->HasBreakpoint(offs);
}

GdbProxy::ResultCode GdbProxy::AddBreakpoint(System::UInt64 offs ) {
  return (GdbProxy::ResultCode)pimpl_->pHost->AddBreakpoint(offs);
}

GdbProxy::ResultCode GdbProxy::RemoveBreakpoint(System::UInt64 offs ) {
  return (GdbProxy::ResultCode)pimpl_->pHost->RemoveBreakpoint(offs);
}

bool GdbProxy::QueryBreakpoint(System::UInt64 offs ) {
  DebugHost::BreakpointRecord dummy;
  return (pimpl_->pHost->QueryBreakpoint(offs, &dummy) == DebugHost::DHR_OK);
}

}}  // namespace NaClVsx.DebugHelpers

