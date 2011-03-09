// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Automatically generated code.  See srpcgen.py
//
// NaCl Simple Remote Procedure Call interface abstractions.

#include "D:\private_svn\nacl\native_client\scons-out\opt-win-x86-32/gen/native_client/src/shared/npruntime/npupcall_rpc.h"
#ifdef __native_client__
#include <nacl/nacl_srpc.h>
#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) do { (void) P; } while (0)
#endif  // UNREFERENCED_PARAMETER
#else
#include "native_client/src/include/portability.h"
#include "native_client/src/shared/srpc/nacl_srpc.h"
#endif  // __native_client__

namespace {

static NaClSrpcError NPN_PluginThreadAsyncCallDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = NPUpcallRpcServer::NPN_PluginThreadAsyncCall(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival
  );
  return retval;
}

static NaClSrpcError Device3DFlushDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPUpcallRpcServer::Device3DFlush(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.ival),
      &(outputs[2]->u.ival)
  );
  return retval;
}

}  // namespace

NACL_SRPC_METHOD_ARRAY(NPUpcallRpcs::srpc_methods) = {
  { "NPN_PluginThreadAsyncCall:ii:", NPN_PluginThreadAsyncCallDispatcher },
  { "Device3DFlush:ii:iii", Device3DFlushDispatcher },
  { NULL, NULL }
};  // NACL_SRPC_METHOD_ARRAY

