// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Automatically generated code.  See srpcgen.py
//
// NaCl Simple Remote Procedure Call interface abstractions.

#include "D:\private_svn\nacl\native_client\scons-out\opt-win-x86-64/gen/native_client/src/shared/npruntime/npnavigator_rpc.h"
#ifdef __native_client__
#include <nacl/nacl_srpc.h>
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) do { (void) P; } while (0)
#endif  // UNREFERENCED_PARAMETER
#else
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#endif  // __native_client__

NaClSrpcError NPNavigatorRpcClient::NP_SetUpcallServices(
    NaClSrpcChannel* channel,
    char* service_string
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NP_SetUpcallServices:s:",
      service_string
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::NP_Initialize(
    NaClSrpcChannel* channel,
    int32_t pid,
    NaClSrpcImcDescType upcall_channel_desc,
    int32_t* nacl_pid
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NP_Initialize:ih:i",
      pid,
      upcall_channel_desc,
      nacl_pid
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::NPP_New(
    NaClSrpcChannel* channel,
    char* mimetype,
    int32_t npp,
    int32_t argc,
    nacl_abi_size_t argn_bytes, char* argn,
    nacl_abi_size_t argv_bytes, char* argv,
    int32_t* nperr
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPP_New:siiCC:i",
      mimetype,
      npp,
      argc,
      argn_bytes, argn,
      argv_bytes, argv,
      nperr
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::NPP_SetWindow(
    NaClSrpcChannel* channel,
    int32_t npp,
    int32_t height,
    int32_t width,
    int32_t* nperr
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPP_SetWindow:iii:i",
      npp,
      height,
      width,
      nperr
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::NPP_Destroy(
    NaClSrpcChannel* channel,
    int32_t npp,
    int32_t* nperr
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPP_Destroy:i:i",
      npp,
      nperr
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::GetScriptableInstance(
    NaClSrpcChannel* channel,
    int32_t npp,
    nacl_abi_size_t* capability_bytes, char* capability
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "GetScriptableInstance:i:C",
      npp,
      capability_bytes, capability
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::NPP_HandleEvent(
    NaClSrpcChannel* channel,
    int32_t npp,
    nacl_abi_size_t npevent_bytes, char* npevent,
    int32_t* return_int16
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPP_HandleEvent:iC:i",
      npp,
      npevent_bytes, npevent,
      return_int16
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::NPP_URLNotify(
    NaClSrpcChannel* channel,
    int32_t npp,
    char* url,
    int32_t reason,
    int32_t notify_data
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPP_URLNotify:isii:",
      npp,
      url,
      reason,
      notify_data
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::NPP_StreamAsFile(
    NaClSrpcChannel* channel,
    int32_t npp,
    NaClSrpcImcDescType handle,
    char* url,
    int32_t size
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPP_StreamAsFile:ihsi:",
      npp,
      handle,
      url,
      size
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::DoAsyncCall(
    NaClSrpcChannel* channel,
    int32_t number
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "DoAsyncCall:i:",
      number
  );
  return retval;
}

NaClSrpcError NPNavigatorRpcClient::AudioCallback(
    NaClSrpcChannel* channel,
    int32_t number,
    NaClSrpcImcDescType shm_desc,
    int32_t shm_size,
    NaClSrpcImcDescType sync_desc
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "AudioCallback:ihih:",
      number,
      shm_desc,
      shm_size,
      sync_desc
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_Deallocate(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_Deallocate:iC:",
      wire_npp,
      capability_bytes, capability
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_Invalidate(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_Invalidate:iC:",
      wire_npp,
      capability_bytes, capability
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_HasMethod(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    int32_t wire_id,
    int32_t* success
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_HasMethod:iCi:i",
      wire_npp,
      capability_bytes, capability,
      wire_id,
      success
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_Invoke(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    int32_t wire_id,
    nacl_abi_size_t args_bytes, char* args,
    int32_t arg_count,
    int32_t* success,
    nacl_abi_size_t* ret_bytes, char* ret
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_Invoke:iCiCi:iC",
      wire_npp,
      capability_bytes, capability,
      wire_id,
      args_bytes, args,
      arg_count,
      success,
      ret_bytes, ret
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_InvokeDefault(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    nacl_abi_size_t args_bytes, char* args,
    int32_t arg_count,
    int32_t* success,
    nacl_abi_size_t* ret_bytes, char* ret
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_InvokeDefault:iCCi:iC",
      wire_npp,
      capability_bytes, capability,
      args_bytes, args,
      arg_count,
      success,
      ret_bytes, ret
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_HasProperty(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    int32_t wire_id,
    int32_t* success
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_HasProperty:iCi:i",
      wire_npp,
      capability_bytes, capability,
      wire_id,
      success
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_GetProperty(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    int32_t wire_id,
    int32_t* success,
    nacl_abi_size_t* ret_bytes, char* ret
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_GetProperty:iCi:iC",
      wire_npp,
      capability_bytes, capability,
      wire_id,
      success,
      ret_bytes, ret
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_SetProperty(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    int32_t wire_id,
    nacl_abi_size_t arg_bytes, char* arg,
    int32_t* success
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_SetProperty:iCiC:i",
      wire_npp,
      capability_bytes, capability,
      wire_id,
      arg_bytes, arg,
      success
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_RemoveProperty(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    int32_t wire_id,
    int32_t* success
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_RemoveProperty:iCi:i",
      wire_npp,
      capability_bytes, capability,
      wire_id,
      success
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_Enumerate(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    int32_t* success,
    nacl_abi_size_t* id_list_bytes, char* id_list,
    int32_t* id_count
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_Enumerate:iC:iCi",
      wire_npp,
      capability_bytes, capability,
      success,
      id_list_bytes, id_list,
      id_count
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_Construct(
    NaClSrpcChannel* channel,
    int32_t wire_npp,
    nacl_abi_size_t capability_bytes, char* capability,
    nacl_abi_size_t args_bytes, char* args,
    int32_t arg_count,
    int32_t* success,
    nacl_abi_size_t* ret_bytes, char* ret
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_Construct:iCCi:iC",
      wire_npp,
      capability_bytes, capability,
      args_bytes, args,
      arg_count,
      success,
      ret_bytes, ret
  );
  return retval;
}

NaClSrpcError NPObjectStubRpcClient::NPN_SetException(
    NaClSrpcChannel* channel,
    nacl_abi_size_t capability_bytes, char* capability,
    char* message
)  {
  NaClSrpcError retval;
  retval = NaClSrpcInvokeBySignature(
      channel,
      "NPN_SetException:Cs:",
      capability_bytes, capability,
      message
  );
  return retval;
}


