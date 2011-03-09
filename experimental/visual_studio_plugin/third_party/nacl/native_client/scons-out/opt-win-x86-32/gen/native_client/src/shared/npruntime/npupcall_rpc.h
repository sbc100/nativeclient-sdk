// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Automatically generated code.  See srpcgen.py
//
// NaCl Simple Remote Procedure Call interface abstractions.

#ifndef GEN_NPRUNTIME_NPUPCALL_RPC_H_
#define GEN_NPRUNTIME_NPUPCALL_RPC_H_
#ifdef __native_client__
#include <nacl/nacl_srpc.h>
#else
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#endif  // __native_client__

class NPUpcallRpcServer {
 public:
  static NaClSrpcError NPN_PluginThreadAsyncCall(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t closure_number
  );
  static NaClSrpcError Device3DFlush(
      NaClSrpcChannel* channel,
      int32_t npp,
      int32_t put_offset,
      int32_t* get_offset,
      int32_t* token,
      int32_t* error
  );

 private:
  NPUpcallRpcServer();
  NPUpcallRpcServer(const NPUpcallRpcServer&);
  void operator=(const NPUpcallRpcServer);

};  // class NPUpcallRpcServer

class NPUpcallRpcs {
 public:
  static NACL_SRPC_METHOD_ARRAY(srpc_methods);
};  // class NPUpcallRpcs

#endif  // GEN_NPRUNTIME_NPUPCALL_RPC_H_

