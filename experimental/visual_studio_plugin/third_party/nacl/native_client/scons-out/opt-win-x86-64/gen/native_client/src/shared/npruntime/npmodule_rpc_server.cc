// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// Automatically generated code.  See srpcgen.py
//
// NaCl Simple Remote Procedure Call interface abstractions.

#include "D:\private_svn\nacl\native_client\scons-out\opt-win-x86-64/gen/native_client/src/shared/npruntime/npmodule_rpc.h"
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

static NaClSrpcError NPN_GetValueBooleanDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_GetValueBoolean(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_GetValueObjectDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_GetValueObject(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.caval.count), outputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_EvaluateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_Evaluate(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.caval.count, inputs[2]->u.caval.carr,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.caval.count), outputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_SetStatusDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_SetStatus(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.sval
  );
  return retval;
}

static NaClSrpcError NPN_GetURLDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_GetURL(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.sval,
      inputs[2]->u.sval,
      inputs[3]->u.ival,
      inputs[4]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_UserAgentDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_UserAgent(
      channel,
      inputs[0]->u.ival,
      &(outputs[0]->u.sval)
  );
  return retval;
}

static NaClSrpcError NPN_GetIntIdentifierDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_GetIntIdentifier(
      channel,
      inputs[0]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_UTF8FromIdentifierDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_UTF8FromIdentifier(
      channel,
      inputs[0]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.sval)
  );
  return retval;
}

static NaClSrpcError NPN_GetStringIdentifierDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_GetStringIdentifier(
      channel,
      inputs[0]->u.sval,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_IntFromIdentifierDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_IntFromIdentifier(
      channel,
      inputs[0]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_IdentifierIsStringDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPModuleRpcServer::NPN_IdentifierIsString(
      channel,
      inputs[0]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_DeallocateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_Deallocate(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_InvalidateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_Invalidate(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_HasMethodDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_HasMethod(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_InvokeDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_Invoke(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.ival,
      inputs[3]->u.caval.count, inputs[3]->u.caval.carr,
      inputs[4]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.caval.count), outputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_InvokeDefaultDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_InvokeDefault(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.caval.count, inputs[2]->u.caval.carr,
      inputs[3]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.caval.count), outputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_HasPropertyDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_HasProperty(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_GetPropertyDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_GetProperty(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.caval.count), outputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_SetPropertyDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_SetProperty(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.ival,
      inputs[3]->u.caval.count, inputs[3]->u.caval.carr,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_RemovePropertyDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_RemoveProperty(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_EnumerateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_Enumerate(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.caval.count), outputs[1]->u.caval.carr,
      &(outputs[2]->u.ival)
  );
  return retval;
}

static NaClSrpcError NPN_ConstructDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_Construct(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.caval.count, inputs[1]->u.caval.carr,
      inputs[2]->u.caval.count, inputs[2]->u.caval.carr,
      inputs[3]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.caval.count), outputs[1]->u.caval.carr
  );
  return retval;
}

static NaClSrpcError NPN_SetExceptionDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = NPObjectStubRpcServer::NPN_SetException(
      channel,
      inputs[0]->u.caval.count, inputs[0]->u.caval.carr,
      inputs[1]->u.sval
  );
  return retval;
}

static NaClSrpcError Device2DInitializeDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = Device2DRpcServer::Device2DInitialize(
      channel,
      inputs[0]->u.ival,
      &(outputs[0]->u.hval),
      &(outputs[1]->u.ival),
      &(outputs[2]->u.ival),
      &(outputs[3]->u.ival),
      &(outputs[4]->u.ival),
      &(outputs[5]->u.ival)
  );
  return retval;
}

static NaClSrpcError Device2DGetStateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = Device2DRpcServer::Device2DGetState(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError Device2DSetStateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = Device2DRpcServer::Device2DSetState(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      inputs[2]->u.ival
  );
  return retval;
}

static NaClSrpcError Device2DFlushDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = Device2DRpcServer::Device2DFlush(
      channel,
      inputs[0]->u.ival,
      &(outputs[0]->u.ival),
      &(outputs[1]->u.ival),
      &(outputs[2]->u.ival),
      &(outputs[3]->u.ival),
      &(outputs[4]->u.ival)
  );
  return retval;
}

static NaClSrpcError Device2DDestroyDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = Device2DRpcServer::Device2DDestroy(
      channel,
      inputs[0]->u.ival
  );
  return retval;
}

static NaClSrpcError Device3DInitializeDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = Device3DRpcServer::Device3DInitialize(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.hval),
      &(outputs[1]->u.ival),
      &(outputs[2]->u.ival),
      &(outputs[3]->u.ival)
  );
  return retval;
}

static NaClSrpcError Device3DDestroyDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = Device3DRpcServer::Device3DDestroy(
      channel,
      inputs[0]->u.ival
  );
  return retval;
}

static NaClSrpcError Device3DGetStateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = Device3DRpcServer::Device3DGetState(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError Device3DSetStateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = Device3DRpcServer::Device3DSetState(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      inputs[2]->u.ival
  );
  return retval;
}

static NaClSrpcError Device3DCreateBufferDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = Device3DRpcServer::Device3DCreateBuffer(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.hval),
      &(outputs[1]->u.ival)
  );
  return retval;
}

static NaClSrpcError Device3DDestroyBufferDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = Device3DRpcServer::Device3DDestroyBuffer(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival
  );
  return retval;
}

static NaClSrpcError AudioInitializeDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = AudioRpcServer::AudioInitialize(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      inputs[2]->u.ival,
      inputs[3]->u.ival,
      inputs[4]->u.ival,
      inputs[5]->u.ival,
      inputs[6]->u.ival,
      inputs[7]->u.ival
  );
  return retval;
}

static NaClSrpcError AudioDestroyDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = AudioRpcServer::AudioDestroy(
      channel,
      inputs[0]->u.ival
  );
  return retval;
}

static NaClSrpcError AudioGetStateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  NaClSrpcError retval;
  retval = AudioRpcServer::AudioGetState(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      &(outputs[0]->u.ival)
  );
  return retval;
}

static NaClSrpcError AudioSetStateDispatcher(
    NaClSrpcChannel* channel,
    NaClSrpcArg** inputs,
    NaClSrpcArg** outputs
) {
  UNREFERENCED_PARAMETER(outputs);
  NaClSrpcError retval;
  retval = AudioRpcServer::AudioSetState(
      channel,
      inputs[0]->u.ival,
      inputs[1]->u.ival,
      inputs[2]->u.ival
  );
  return retval;
}

}  // namespace

NACL_SRPC_METHOD_ARRAY(NPModuleRpcs::srpc_methods) = {
  { "NPN_GetValueBoolean:ii:ii", NPN_GetValueBooleanDispatcher },
  { "NPN_GetValueObject:ii:iC", NPN_GetValueObjectDispatcher },
  { "NPN_Evaluate:iCC:iC", NPN_EvaluateDispatcher },
  { "NPN_SetStatus:is:", NPN_SetStatusDispatcher },
  { "NPN_GetURL:issii:i", NPN_GetURLDispatcher },
  { "NPN_UserAgent:i:s", NPN_UserAgentDispatcher },
  { "NPN_GetIntIdentifier:i:i", NPN_GetIntIdentifierDispatcher },
  { "NPN_UTF8FromIdentifier:i:is", NPN_UTF8FromIdentifierDispatcher },
  { "NPN_GetStringIdentifier:s:i", NPN_GetStringIdentifierDispatcher },
  { "NPN_IntFromIdentifier:i:i", NPN_IntFromIdentifierDispatcher },
  { "NPN_IdentifierIsString:i:i", NPN_IdentifierIsStringDispatcher },
  { "NPN_Deallocate:iC:", NPN_DeallocateDispatcher },
  { "NPN_Invalidate:iC:", NPN_InvalidateDispatcher },
  { "NPN_HasMethod:iCi:i", NPN_HasMethodDispatcher },
  { "NPN_Invoke:iCiCi:iC", NPN_InvokeDispatcher },
  { "NPN_InvokeDefault:iCCi:iC", NPN_InvokeDefaultDispatcher },
  { "NPN_HasProperty:iCi:i", NPN_HasPropertyDispatcher },
  { "NPN_GetProperty:iCi:iC", NPN_GetPropertyDispatcher },
  { "NPN_SetProperty:iCiC:i", NPN_SetPropertyDispatcher },
  { "NPN_RemoveProperty:iCi:i", NPN_RemovePropertyDispatcher },
  { "NPN_Enumerate:iC:iCi", NPN_EnumerateDispatcher },
  { "NPN_Construct:iCCi:iC", NPN_ConstructDispatcher },
  { "NPN_SetException:Cs:", NPN_SetExceptionDispatcher },
  { "Device2DInitialize:i:hiiiii", Device2DInitializeDispatcher },
  { "Device2DGetState:ii:i", Device2DGetStateDispatcher },
  { "Device2DSetState:iii:", Device2DSetStateDispatcher },
  { "Device2DFlush:i:iiiii", Device2DFlushDispatcher },
  { "Device2DDestroy:i:", Device2DDestroyDispatcher },
  { "Device3DInitialize:ii:hiii", Device3DInitializeDispatcher },
  { "Device3DDestroy:i:", Device3DDestroyDispatcher },
  { "Device3DGetState:ii:i", Device3DGetStateDispatcher },
  { "Device3DSetState:iii:", Device3DSetStateDispatcher },
  { "Device3DCreateBuffer:ii:hi", Device3DCreateBufferDispatcher },
  { "Device3DDestroyBuffer:ii:", Device3DDestroyBufferDispatcher },
  { "AudioInitialize:iiiiiiii:", AudioInitializeDispatcher },
  { "AudioDestroy:i:", AudioDestroyDispatcher },
  { "AudioGetState:ii:i", AudioGetStateDispatcher },
  { "AudioSetState:iii:", AudioSetStateDispatcher },
  { NULL, NULL }
};  // NACL_SRPC_METHOD_ARRAY

