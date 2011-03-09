/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "native_client/src/trusted/plugin/srpc/multimedia_socket.h"

#include <string.h>

#include "native_client/src/include/portability_io.h"
#include "native_client/src/shared/platform/nacl_sync.h"
#include "native_client/src/shared/platform/nacl_sync_checked.h"
#include "native_client/src/shared/platform/nacl_threads.h"
#include "native_client/src/shared/platform/nacl_time.h"

#include "native_client/src/trusted/desc/nacl_desc_imc.h"

#include "native_client/src/trusted/plugin/srpc/plugin.h"
#include "native_client/src/trusted/plugin/srpc/service_runtime_interface.h"
#include "native_client/src/trusted/plugin/srpc/shared_memory.h"
#include "native_client/src/trusted/plugin/srpc/srpc_client.h"
#include "native_client/src/trusted/plugin/srpc/utility.h"
#include "native_client/src/trusted/plugin/srpc/video.h"

namespace nacl_srpc {

uintptr_t MultimediaSocket::kNaClMultimediaBridgeIdent;

// NB: InitializeIdentifiers is not thread-safe.
void MultimediaSocket::InitializeIdentifiers(
    PortablePluginInterface *plugin_interface) {
  static bool identifiers_initialized = false;

  UNREFERENCED_PARAMETER(plugin_interface);
  if (!identifiers_initialized) {
    kNaClMultimediaBridgeIdent =
        PortablePluginInterface::GetStrIdentifierCallback(
          "nacl_multimedia_bridge");
    identifiers_initialized = true;
  }
}

MultimediaSocket::MultimediaSocket(ScriptableHandle<ConnectedSocket>* s,
                                   PortablePluginInterface* plugin_interface,
                                   ServiceRuntimeInterface* sri)
    : connected_socket_(s),
      plugin_interface_(plugin_interface),
      service_runtime_(sri),
      upcall_thread_should_exit_(false),
      upcall_thread_id_(0) {
  connected_socket_->AddRef();
  NaClMutexCtor(&mu_);
  NaClCondVarCtor(&cv_);
  upcall_thread_state_ = UPCALL_THREAD_NOT_STARTED;
  InitializeIdentifiers(plugin_interface_);  // inlineable; tail call.
}

MultimediaSocket::~MultimediaSocket() {
  enum UpcallThreadState ts;
  struct nacl_abi_timeval now;
  struct nacl_abi_timespec giveup;
  bool murdered(false);

  dprintf(("MultimediaSocket::~MultimediaSocket: entered\n"));
  NaClXMutexLock(&mu_);
  if (NULL == getenv("NACLTEST_DISABLE_SHUTDOWN")) {
    upcall_thread_should_exit_ = true;
    dprintf((" set flag to tell upcall thread to exit.\n"));
  } else {
    dprintf((" NOT telling upcall thread to exit.\n"));
  }
  dprintf((" upcall_thread_state_ %d\n", upcall_thread_state_));
  if (UPCALL_THREAD_NOT_STARTED != (ts = upcall_thread_state_)) {
    dprintf((" computing giveup time.\n"));
    NaClGetTimeOfDay(&now);
    giveup.tv_sec = now.nacl_abi_tv_sec + kMaxUpcallThreadWaitSec;
    giveup.tv_nsec = now.nacl_abi_tv_usec * kNanoXinMicroX;
    while (UPCALL_THREAD_EXITED != upcall_thread_state_) {
      dprintf(("MultimediaSocket::~MultimediaSocket:"
               " waiting for upcall thread to exit\n"));
      if (!murdered) {
        if (NACL_SYNC_CONDVAR_TIMEDOUT ==
            NaClCondVarTimedWaitAbsolute(&cv_, &mu_, &giveup)) {
          dprintf(("MultimediaSocket::~MultimediaSocket:"
                   " timed out, killing service runtime process\n"));
          if (!service_runtime_->Kill()) {
            // We try our best, but if KillChild fails, we just keep
            // going.
            dprintf(("Could not kill child!\n"));
          }
          murdered = true;
        }
      } else {
        // wait for the process kill to cause the sockets to close
        // etc, and thus cause the upcall thread to exit due to EOF.
        NaClXCondVarWait(&cv_, &mu_);
      }
    }
  }
  NaClXMutexUnlock(&mu_);
  connected_socket_->Unref();
  if (UPCALL_THREAD_NOT_STARTED != ts) {
    NaClThreadDtor(&upcall_thread_);
  }
  NaClCondVarDtor(&cv_);
  NaClMutexDtor(&mu_);
  dprintf(("MultimediaSocket::~MultimediaSocket: done.\n"));
}

void MultimediaSocket::UpcallThreadExiting() {
  NaClXMutexLock(&mu_);
  upcall_thread_state_ = UPCALL_THREAD_EXITED;
  NaClXCondVarBroadcast(&cv_);
  NaClXMutexUnlock(&mu_);
}

static NaClSrpcError handleUpcall(NaClSrpcChannel* channel,
                                  NaClSrpcArg** ins,
                                  NaClSrpcArg** outs) {
  NaClSrpcError ret;

  UNREFERENCED_PARAMETER(ins);
  UNREFERENCED_PARAMETER(outs);
  ret = NACL_SRPC_RESULT_BREAK;
  if (channel) {
    nacl::VideoScopedGlobalLock video_lock;
    nacl::VideoCallbackData *video_cb_data;
    PortablePluginInterface *plugin_interface;
    MultimediaSocket *msp;

    dprintf(("Upcall: channel %p\n", static_cast<void *>(channel)));
    dprintf(("Upcall: server_instance_data %p\n",
             static_cast<void *>(channel->server_instance_data)));
    video_cb_data = reinterpret_cast<nacl::VideoCallbackData*>
        (channel->server_instance_data);
    plugin_interface = video_cb_data->portable_plugin;
    if (NULL != plugin_interface) {
      nacl::VideoMap *video = plugin_interface->video();
      if (video) {
        video->RequestRedraw();
      }
      ret = NACL_SRPC_RESULT_OK;
    } else if (NULL != getenv("NACLTEST_DISABLE_SHUTDOWN")) {
      dprintf(("Upcall: SrpcPlugin dtor invoked VideoMap dtor, but pretending"
               " that the channel is okay for testing\n"));
      ret = NACL_SRPC_RESULT_OK;
    } else {
      dprintf(("Upcall: plugin was NULL\n"));
    }
    msp = video_cb_data->msp;
    if (NULL != msp) {
      if (msp->UpcallThreadShouldExit()) {
        ret = NACL_SRPC_RESULT_BREAK;
      }
    }
  }
  if (NACL_SRPC_RESULT_OK == ret) {
    dprintf(("Upcall: success\n"));
  } else if (NACL_SRPC_RESULT_BREAK == ret) {
    dprintf(("Upcall: break detected, thread exiting\n"));
  } else {
    dprintf(("Upcall: failure\n"));
  }
  return ret;
}

bool MultimediaSocket::UpcallThreadShouldExit() {
  bool should_exit;
  NaClMutexLock(&mu_);
  should_exit = upcall_thread_should_exit_;
  NaClMutexUnlock(&mu_);
  return should_exit;
}

void MultimediaSocket::set_upcall_thread_id(uint32_t tid) {
  NaClMutexLock(&mu_);
  upcall_thread_id_ = tid;
  NaClCondVarSignal(&cv_);
  NaClMutexUnlock(&mu_);
}

static void WINAPI UpcallThread(void *arg) {
  nacl::VideoCallbackData *cbdata;
  NaClSrpcHandlerDesc handlers[] = {
    { "upcall::", handleUpcall },
    { NULL, NULL }
  };

  cbdata = reinterpret_cast<nacl::VideoCallbackData*>(arg);
  dprintf(("MultimediaSocket::UpcallThread(%p)\n", arg));
  dprintf(("MultimediaSocket::cbdata->portable_plugin %p\n",
           static_cast<void *>(cbdata->portable_plugin)));
  // Set up the DescWrapper* the server will be placed on.
  if (NULL == cbdata->handle) {
    dprintf(("MultimediaSocket::UpcallThread(%p) FAILED\n", arg));
    return;
  }
  cbdata->msp->set_upcall_thread_id(NaClThreadId());
  // Run the SRPC server.
  NaClSrpcServerLoop(cbdata->handle->desc(), handlers, cbdata);
  // release the cbdata
  cbdata->msp->UpcallThreadExiting();
  nacl::VideoGlobalLock();
  nacl::VideoMap::ReleaseCallbackData(cbdata);
  nacl::VideoGlobalUnlock();
  dprintf(("MultimediaSocket::UpcallThread: End\n"));
}

// Support for initializing the NativeClient multimedia system.
bool MultimediaSocket::InitializeModuleMultimedia(Plugin *plugin) {
  dprintf(("MultimediaSocket::InitializeModuleMultimedia(%p)\n",
           static_cast<void *>(this)));

  PortablePluginInterface *plugin_interface =
      connected_socket_->get_handle()->GetPortablePluginInterface();
  nacl::VideoMap *video = plugin_interface->video();
  ScriptableHandle<SharedMemory> *video_shared_memory =
      video->VideoSharedMemorySetup();

  // If there is no display shared memory region, don't initialize.
  // TODO(nfullagar,sehr): make sure this makes sense with NPAPI call order.
  if (NULL == video_shared_memory) {
    dprintf(("MultimediaSocket::InitializeModuleMultimedia:"
             "No video_shared_memory.\n"));
    // trivial success case.  NB: if NaCl module and HTML disagrees,
    // then the HTML wins.
    return true;
  }
  // Determine whether the NaCl module has reported having a method called
  // "nacl_multimedia_bridge".
  if (!(connected_socket()->HasMethod(kNaClMultimediaBridgeIdent,
                                      METHOD_CALL))) {
    dprintf(("No nacl_multimedia_bridge method was found.\n"));
    return false;
  }
  // Create a socket pair.
  nacl::DescWrapper* desc[2];
  if (0 != plugin->wrapper_factory()->MakeSocketPair(desc)) {
    dprintf(("MakeSocketPair failed!\n"));
    return false;
  }
  // Start a thread to handle the upcalls.
  nacl::VideoCallbackData *cbdata;
  cbdata = video->InitCallbackData(desc[0], plugin_interface, this);
  dprintf((
      "MultimediaSocket::InitializeModuleMultimedia: launching thread\n"));
  uint32_t tid;
  NaClXMutexLock(&mu_);
  if (upcall_thread_state_ != UPCALL_THREAD_NOT_STARTED) {
    dprintf(("Internal error: upcall thread already running\n"));
    NaClXMutexUnlock(&mu_);
    return false;
  }
  if (!NaClThreadCtor(&upcall_thread_, UpcallThread, cbdata, 128 << 10)) {
    NaClXMutexUnlock(&mu_);
    // not constructed, so no dtor will be needed
    video->ForceDeleteCallbackData(cbdata);
    return false;
  }
  upcall_thread_state_ = UPCALL_THREAD_RUNNING;

  while (0 == (tid = upcall_thread_id_)) {
    NaClXCondVarWait(&cv_, &mu_);
  }
  NaClXMutexUnlock(&mu_);

  char buf[512];
  SNPRINTF(buf, sizeof buf,
           "MultimediaSocket upcall thread %x (%d)",
           tid,
           tid);
  service_runtime_->LogAtServiceRuntime(4, buf);

  SrpcParams params("oo", "");

  params.Input(0)->tag = NACL_SRPC_ARG_TYPE_HANDLE;
  SharedMemory* internal_video_shared_memory =
      static_cast<SharedMemory*>(video_shared_memory->get_handle());
  params.Input(0)->u.hval = internal_video_shared_memory->desc();
  params.Input(1)->tag = NACL_SRPC_ARG_TYPE_HANDLE;
  params.Input(1)->u.hval = desc[1]->desc();

  dprintf(("CS:IMM params %p\n", static_cast<void *>(&params)));

  bool rpc_result = (connected_socket()->Invoke(kNaClMultimediaBridgeIdent,
                                                METHOD_CALL,
                                                &params));
  dprintf(("CS:IMM returned %d\n", static_cast<int>(rpc_result)));
  desc[1]->Delete();

  return rpc_result;
}

}  // namespace nacl_srpc
