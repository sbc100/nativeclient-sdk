// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Automatically generated code.  See srpcgen.py
//
// NaCl Simple Remote Procedure Call interface abstractions.

#ifndef GEN_NPRUNTIME_NPNAVIGATOR_RPC_H_
#define GEN_NPRUNTIME_NPNAVIGATOR_RPC_H_
#ifdef __native_client__
#include <nacl/nacl_srpc.h>
#else
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#endif  // __native_client__

class NPNavigatorRpcClient {
 public:
  static NaClSrpcError NP_SetUpcallServices(
      NaClSrpcChannel* channel,
      char* service_string
  );
  static NaClSrpcError NP_Initialize(
      NaClSrpcChannel* channel,
      int32_t pid,
      NaClSrpcImcDescType upcall_channel_desc,
      int32_t* nacl_pid
  );
  static NaClSrpcError NPP_New(
      NaClSrpcChannel* channel,
      char* mimetype,
      int32_t npp,
      int32_t argc,
      nacl_abi_size_t argn_bytes, char* argn,
      nacl_abi_size_t argv_bytes, char* argv,
      int32_t* nperr
  );
  static NaClSrpcError NPP_SetWindow(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t height,
      int32_t width,
      int32_t* nperr
  );
  static NaClSrpcError NPP_Destroy(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t* nperr
  );
  static NaClSrpcError GetScriptableInstance(
      NaClSrpcChannel* channel,
      int32_t npp,
      nacl_abi_size_t* capability_bytes, char* capability
  );
  static NaClSrpcError NPP_HandleEvent(
      NaClSrpcChannel* channel,
      int32_t npp,
      nacl_abi_size_t npevent_bytes, char* npevent,
      int32_t* return_int16
  );
  static NaClSrpcError NPP_URLNotify(
      NaClSrpcChannel* channel,
      int32_t npp,
      char* url,
      int32_t reason,
      int32_t notify_data
  );
  static NaClSrpcError NPP_StreamAsFile(
      NaClSrpcChannel* channel,
      int32_t npp,
      NaClSrpcImcDescType handle,
      char* url,
      int32_t size
  );
  static NaClSrpcError DoAsyncCall(
      NaClSrpcChannel* channel,
      int32_t number
  );
  static NaClSrpcError AudioCallback(
      NaClSrpcChannel* channel,
      int32_t number,
      NaClSrpcImcDescType shm_desc,
      int32_t shm_size,
      NaClSrpcImcDescType sync_desc
  );

 private:
  NPNavigatorRpcClient();
  NPNavigatorRpcClient(const NPNavigatorRpcClient&);
  void operator=(const NPNavigatorRpcClient);

};  // class NPNavigatorRpcClient

class NPObjectStubRpcClient {
 public:
  static NaClSrpcError NPN_Deallocate(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability
  );
  static NaClSrpcError NPN_Invalidate(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability
  );
  static NaClSrpcError NPN_HasMethod(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      int32_t wire_id,
      int32_t* success
  );
  static NaClSrpcError NPN_Invoke(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      int32_t wire_id,
      nacl_abi_size_t args_bytes, char* args,
      int32_t arg_count,
      int32_t* success,
      nacl_abi_size_t* ret_bytes, char* ret
  );
  static NaClSrpcError NPN_InvokeDefault(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      nacl_abi_size_t args_bytes, char* args,
      int32_t arg_count,
      int32_t* success,
      nacl_abi_size_t* ret_bytes, char* ret
  );
  static NaClSrpcError NPN_HasProperty(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      int32_t wire_id,
      int32_t* success
  );
  static NaClSrpcError NPN_GetProperty(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      int32_t wire_id,
      int32_t* success,
      nacl_abi_size_t* ret_bytes, char* ret
  );
  static NaClSrpcError NPN_SetProperty(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      int32_t wire_id,
      nacl_abi_size_t arg_bytes, char* arg,
      int32_t* success
  );
  static NaClSrpcError NPN_RemoveProperty(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      int32_t wire_id,
      int32_t* success
  );
  static NaClSrpcError NPN_Enumerate(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      int32_t* success,
      nacl_abi_size_t* id_list_bytes, char* id_list,
      int32_t* id_count
  );
  static NaClSrpcError NPN_Construct(
      NaClSrpcChannel* channel,
      int32_t wire_npp,
      nacl_abi_size_t capability_bytes, char* capability,
      nacl_abi_size_t args_bytes, char* args,
      int32_t arg_count,
      int32_t* success,
      nacl_abi_size_t* ret_bytes, char* ret
  );
  static NaClSrpcError NPN_SetException(
      NaClSrpcChannel* channel,
      nacl_abi_size_t capability_bytes, char* capability,
      char* message
  );

 private:
  NPObjectStubRpcClient();
  NPObjectStubRpcClient(const NPObjectStubRpcClient&);
  void operator=(const NPObjectStubRpcClient);

};  // class NPObjectStubRpcClient



#endif  // GEN_NPRUNTIME_NPNAVIGATOR_RPC_H_

