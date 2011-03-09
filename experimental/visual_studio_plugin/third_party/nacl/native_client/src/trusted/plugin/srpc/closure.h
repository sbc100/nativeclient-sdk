/*
 * Copyright 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */


#ifndef NATIVE_CLIENT_SRC_TRUSTED_PLUGIN_SRPC_CLOSURE_H_
#define NATIVE_CLIENT_SRC_TRUSTED_PLUGIN_SRPC_CLOSURE_H_

#include "native_client/src/include/nacl_string.h"
#include "native_client/src/shared/npruntime/npmodule.h"
#include "native_client/src/trusted/plugin/npinstance.h"
#include "native_client/src/trusted/plugin/srpc/plugin.h"

namespace nacl {
  class StreamShmBuffer;
}

namespace nacl_srpc {

/*
 * Closure base class.  Following our C++ style rules, we have
 * an explicit Run method rather than overload the operator().
 *
 * This is pretty generic, actually, and not restricted to be used for
 * notification callbacks.
 */
class Closure {
 public:
  Closure(Plugin* plugin, nacl::string requested_url) :
    plugin_(plugin), requested_url_(requested_url), buffer_(NULL) {
    if (NULL != plugin_) {
      plugin_identifier_ =
          plugin_->GetPortablePluginInterface()->GetPluginIdentifier();
    }
  }
  virtual ~Closure() {}
  virtual void Run(NPStream *stream, const char *fname) = 0;
  virtual void Run(const char *url, nacl::StreamShmBuffer* shmbufp) = 0;
  bool StartDownload();
  void set_plugin(nacl_srpc::Plugin* plugin) {
    plugin_ = plugin;
    plugin_identifier_ =
        plugin_->GetPortablePluginInterface()->GetPluginIdentifier();
  }
  void set_buffer(nacl::StreamShmBuffer* buffer) { buffer_ = buffer; }
  nacl::StreamShmBuffer* buffer() { return buffer_; }
 protected:
  Plugin* plugin() { return plugin_; }
  nacl::string requested_url() { return requested_url_; }
 private:
  Plugin* plugin_;
  nacl::string requested_url_;
  nacl::StreamShmBuffer *buffer_;
  nacl_srpc::PluginIdentifier plugin_identifier_;
};

class LoadNaClAppNotify : public Closure {
 public:
  LoadNaClAppNotify(Plugin *plugin, nacl::string url);
  virtual ~LoadNaClAppNotify();
  virtual void Run(NPStream* stream, const char* fname);
  virtual void Run(const char *url, nacl::StreamShmBuffer* shmbufp);
};

class UrlAsNaClDescNotify : public Closure {
 public:
  UrlAsNaClDescNotify(Plugin *plugin, nacl::string url, void *callback_obj);
  virtual ~UrlAsNaClDescNotify();
  virtual void Run(NPStream *stream, const char *fname);
  virtual void Run(const char *url, nacl::StreamShmBuffer* shmbufp);
 private:
  NPObject* np_callback_;
};

class NpGetUrlClosure : public Closure {
 public:
  NpGetUrlClosure(NPP npp,
                  nacl::NPModule* module,
                  nacl::string url,
                  int32_t notify_data,
                  bool call_url_notify);
  virtual ~NpGetUrlClosure();
  virtual void Run(NPStream* stream, const char* fname);
  virtual void Run(const char* url, nacl::StreamShmBuffer* shmbufp);

 private:
  nacl::NPModule* module_;
  NPP npp_;
  int32_t notify_data_;
  bool call_url_notify_;
};
}  // namespace nacl_srpc

#endif  // NATIVE_CLIENT_SRC_TRUSTED_PLUGIN_SRPC_CLOSURE_H_
