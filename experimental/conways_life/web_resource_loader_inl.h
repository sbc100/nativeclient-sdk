// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_RESOURCE_LOADER_INL_H_
#define WEB_RESOURCE_LOADER_INL_H_

#include "experimental/conways_life/web_resource_loader.h"

#include <stdio.h>
#include <cassert>
#include <string>
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/url_response_info.h"

namespace life {

template <class Delegate>
WebResourceLoader<Delegate>::WebResourceLoader(pp::Instance* instance,
                                               Delegate* delegate)
  : factory_(this),
    instance_(instance),
    url_loader_(*instance),
    connected_(false),
    buffer_(NULL),
    buffer_size_(0),
    data_size_(0),
    delegate_(delegate) {
  assert(delegate != NULL);
}

template <class Delegate>
void WebResourceLoader<Delegate>::CloseAndDeleteSelf() {
  // Can't close & delete this instance here. Instead, we schedule a main-thread
  // callback to delete it later.
  pp::CompletionCallback cc(DeleteOnMainThread, this);
  pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
}

template <class Delegate>
WebResourceLoader<Delegate>::~WebResourceLoader() {
  Close();
}

template <class Delegate>
void WebResourceLoader<Delegate>::LoadURL(const std::string& url) {
  // Check that there is no pending request.
  assert(!connected_);
  assert(url_loader_.GetResponseInfo().is_null());
  // Only usable from main plugin thread.
  pp::CompletionCallback cc = factory_.NewCallback(
      &WebResourceLoader<Delegate>::StartDownload, std::string(url));
  pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
}

template <class Delegate>
const std::string WebResourceLoader<Delegate>::url() const {
  pp::URLResponseInfo response = url_loader_.GetResponseInfo();
  return response.GetURL().AsString();
}

template <class Delegate>
int32_t WebResourceLoader<Delegate>::GetHttpStatus() const {
  return GetResponseInfo().GetStatusCode();
}

template <class Delegate>
pp::URLResponseInfo WebResourceLoader<Delegate>::GetResponseInfo() const {
  pp::URLResponseInfo response = url_loader_.GetResponseInfo();
  assert(!response.is_null());
  return response;
}

template <class Delegate>
void WebResourceLoader<Delegate>::Close() {
  assert(pp::Module::Get()->core()->IsMainThread());
  connected_ = false;
  buffer_ = NULL;
  buffer_size_ = 0;
  data_size_ = 0;
}

template <class Delegate>
void WebResourceLoader<Delegate>::CompletionCallbackFunc(
    int32_t result, DispatchOpCode op_code) {
  if (result < 0) {
    // A negative value indicates an error.
    delegate_->OnWebResourceLoaderError(result, this);
    delegate_->OnWebResourceLoaderDone(this);
  } else if (result == PP_OK && !connected_) {
    assert(op_code == kUrlResponseInfoReady);
    // If we're not yet connected and get result == PP_OK, then we received
    // the http headers.
    connected_ = true;
    pp::URLResponseInfo response = url_loader_.GetResponseInfo();
    if (delegate_->OnWebResourceLoaderCallback(kUrlResponseInfoReady, this)) {
        ReadNextDataBlock();
    } else {
      delegate_->OnWebResourceLoaderDone(this);
    }
  } else {
    assert(op_code == kDataReceived);
    if (result == 0) {
      // No more data; the download completed successfully.
      data_size_ = 0;
      delegate_->OnWebResourceLoaderCallback(kDownloadComplete, this);
      delegate_->OnWebResourceLoaderDone(this);
    } else {
      // We received a block of data of size |result|.
      data_size_ = result;
      if (delegate_->OnWebResourceLoaderCallback(kDataReceived, this)) {
        ReadNextDataBlock();
      } else {
        delegate_->OnWebResourceLoaderDone(this);
      }
    }
  }
}

template <class Delegate>
pp::CompletionCallback WebResourceLoader<Delegate>::MakeCallback(
    DispatchOpCode op_code) {
  return factory_.NewCallback(
      &WebResourceLoader<Delegate>::CompletionCallbackFunc, op_code);
}

template <class Delegate>
void WebResourceLoader<Delegate>::InitializeRequest(
    const std::string& url,
    pp::URLRequestInfo* request) {
  request->SetURL(url);
  request->SetMethod("GET");
  request->SetFollowRedirects(true);
  request->SetStreamToFile(false);
}

template <class Delegate>
void WebResourceLoader<Delegate>::ReadNextDataBlock() {
  // If a custom buffer has not been set or has been set to NULL, use the
  // internal buffer.
  if (buffer_ == NULL) {
    buffer_ = internal_buffer_;
    buffer_size_ = kInternalBufferSize;
  }
  // Get the next block of data.
  pp::CompletionCallback cc = MakeCallback(kDataReceived);
  int32_t rv = url_loader_.ReadResponseBody(buffer_, buffer_size_, cc);
  if (rv != PP_OK_COMPLETIONPENDING) {
    cc.Run(rv);
  }
}

template <class Delegate>
void WebResourceLoader<Delegate>::StartDownload(int32_t result,
                                                const std::string& url) {
  // Only usable from main plugin thread.
  assert(pp::Module::Get()->core()->IsMainThread());

  pp::CompletionCallback cc = MakeCallback(kUrlResponseInfoReady);
  pp::URLRequestInfo request(instance_);
  InitializeRequest(url, &request);
  int32_t rv = url_loader_.Open(request, cc);
  if (rv != PP_OK_COMPLETIONPENDING) {
    // Getting here isn't necessarily an error. It indicates that the call to
    // Open did not take place asynchronously. That can happen, for instance,
    // when the resource data comes out of the cache. In any case, we simply
    // invoke the callback 'manually.'
    cc.Run(rv);
  }
}

template <class Delegate>
void WebResourceLoader<Delegate>::DeleteOnMainThread(void* user_data,
                                                     int32_t err) {
  WebResourceLoader* loader = reinterpret_cast<WebResourceLoader*>(user_data);
  if (loader != NULL) delete loader;
}

}  // namespace life

#endif  // WEB_RESOURCE_LOADER_INL_H_
