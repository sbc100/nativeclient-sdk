// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Automatically generated code.  See srpcgen.py
//
// NaCl Simple Remote Procedure Call interface abstractions.

#ifndef GEN_NPRUNTIME_NPMODULE_RPC_H_
#define GEN_NPRUNTIME_NPMODULE_RPC_H_
#ifdef __native_client__
#include <nacl/nacl_srpc.h>
#else
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#endif  // __native_client__

class NPModuleRpcServer {
 public:
  static NaClSrpcError NPN_GetValueBoolean(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t var,
      int32_t* nperr,
      int32_t* result
  );
  static NaClSrpcError NPN_GetValueObject(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t var,
      int32_t* nperr,
      nacl_abi_size_t* result_bytes, char* result
  );
  static NaClSrpcError NPN_Evaluate(
      NaClSrpcChannel* channel,
      int32_t npp,
      nacl_abi_size_t obj_bytes, char* obj,
      nacl_abi_size_t str_bytes, char* str,
      int32_t* nperr,
      nacl_abi_size_t* result_bytes, char* result
  );
  static NaClSrpcError NPN_SetStatus(
      NaClSrpcChannel* channel,
      int32_t npp,
      char* status
  );
  static NaClSrpcError NPN_GetURL(
      NaClSrpcChannel* channel,
      int32_t npp,
      char* url,
      char* target,
      int32_t notify_data,
      int32_t call_url_notify,
      int32_t* success
  );
  static NaClSrpcError NPN_UserAgent(
      NaClSrpcChannel* channel,
      int32_t npp,
      char** strval
  );
  static NaClSrpcError NPN_GetIntIdentifier(
      NaClSrpcChannel* channel,
      int32_t intval,
      int32_t* id
  );
  static NaClSrpcError NPN_UTF8FromIdentifier(
      NaClSrpcChannel* channel,
      int32_t id,
      int32_t* success,
      char** str
  );
  static NaClSrpcError NPN_GetStringIdentifier(
      NaClSrpcChannel* channel,
      char* strval,
      int32_t* id
  );
  static NaClSrpcError NPN_IntFromIdentifier(
      NaClSrpcChannel* channel,
      int32_t id,
      int32_t* intval
  );
  static NaClSrpcError NPN_IdentifierIsString(
      NaClSrpcChannel* channel,
      int32_t id,
      int32_t* isstring
  );

 private:
  NPModuleRpcServer();
  NPModuleRpcServer(const NPModuleRpcServer&);
  void operator=(const NPModuleRpcServer);

};  // class NPModuleRpcServer

class NPObjectStubRpcServer {
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
  NPObjectStubRpcServer();
  NPObjectStubRpcServer(const NPObjectStubRpcServer&);
  void operator=(const NPObjectStubRpcServer);

};  // class NPObjectStubRpcServer

class Device2DRpcServer {
 public:
  static NaClSrpcError Device2DInitialize(
      NaClSrpcChannel* channel,
      int32_t npp,
      NaClSrpcImcDescType* shared_memory_desc,
      int32_t* stride,
      int32_t* left,
      int32_t* top,
      int32_t* right,
      int32_t* bottom
  );
  static NaClSrpcError Device2DGetState(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t state,
      int32_t* value
  );
  static NaClSrpcError Device2DSetState(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t state,
      int32_t value
  );
  static NaClSrpcError Device2DFlush(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t* stride,
      int32_t* left,
      int32_t* top,
      int32_t* right,
      int32_t* bottom
  );
  static NaClSrpcError Device2DDestroy(
      NaClSrpcChannel* channel,
      int32_t npp
  );

 private:
  Device2DRpcServer();
  Device2DRpcServer(const Device2DRpcServer&);
  void operator=(const Device2DRpcServer);

};  // class Device2DRpcServer

class Device3DRpcServer {
 public:
  static NaClSrpcError Device3DInitialize(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t command_buffer_entries_requested,
      NaClSrpcImcDescType* shared_memory_desc,
      int32_t* command_buffer_entries_obtained,
      int32_t* get_offset,
      int32_t* put_offset
  );
  static NaClSrpcError Device3DDestroy(
      NaClSrpcChannel* channel,
      int32_t npp
  );
  static NaClSrpcError Device3DGetState(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t state,
      int32_t* value
  );
  static NaClSrpcError Device3DSetState(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t state,
      int32_t value
  );
  static NaClSrpcError Device3DCreateBuffer(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t size,
      NaClSrpcImcDescType* shared_memory_desc,
      int32_t* id
  );
  static NaClSrpcError Device3DDestroyBuffer(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t id
  );

 private:
  Device3DRpcServer();
  Device3DRpcServer(const Device3DRpcServer&);
  void operator=(const Device3DRpcServer);

};  // class Device3DRpcServer

class AudioRpcServer {
 public:
  static NaClSrpcError AudioInitialize(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t number,
      int32_t sample_rate,
      int32_t sample_type,
      int32_t output_channel_map,
      int32_t input_channel_map,
      int32_t sample_frame_count,
      int32_t flags
  );
  static NaClSrpcError AudioDestroy(
      NaClSrpcChannel* channel,
      int32_t npp
  );
  static NaClSrpcError AudioGetState(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t state,
      int32_t* value
  );
  static NaClSrpcError AudioSetState(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t state,
      int32_t value
  );

 private:
  AudioRpcServer();
  AudioRpcServer(const AudioRpcServer&);
  void operator=(const AudioRpcServer);

};  // class AudioRpcServer

class NPModuleRpcs {
 public:
  static NACL_SRPC_METHOD_ARRAY(srpc_methods);
};  // class NPModuleRpcs

#endif  // GEN_NPRUNTIME_NPMODULE_RPC_H_

