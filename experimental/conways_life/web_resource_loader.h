// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_RESOURCE_LOADER_H_
#define WEB_RESOURCE_LOADER_H_

#include <string>

#include "experimental/conways_life/threading/ref_count.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/cpp/url_loader.h"

namespace pp {
class Instance;
class URLRequestInfo;
}  // namespace pp

namespace life {

// WebResourceLoader handles downloading resources from URLs. It handles the
// callbacks. It uses a delegate pattern to connect to the class that
// is aggregating and interpreting the resource data. This Delegate class must
// declare the following methods:
//
// OnWebResourceLoaderCallback:
// Called at various times during the download process. The op_code indicates
// what step has just been completed. The Delegate should return true to
// continue the download process, or false to abort.
//     bool OnWebResourceLoaderCallback(DispatchOpCode op_code,
//                                      WebResourceLoader* loader);
//
// OnWebResourceLoaderError:
// Called when an error occurs.
//     void OnWebResourceLoaderError(int32_t error, WebResourceLoader* loader);
//
// OnWebResourceLoaderDone:
// This is always the last method called, either after a successful download or
// after an error.
//     void OnWebResourceLoaderDone(WebResourceLoader* loader);
template <class Delegate>
class WebResourceLoader {
 public:
  // DispatchOpCode used with Delegate::OnWebResourceLoaderCallback.
  enum DispatchOpCode {
    kUrlResponseInfoReady,  // URL response ready; query with GetResponseInfo.
    kDataReceived,  // A chunk of data has been received in buffer.
    kDownloadComplete  // Download done, data/file is ready for consumption.
  };

  WebResourceLoader(pp::Instance* instance, Delegate* delegate);
  virtual ~WebResourceLoader();

  // You can call this function from any thread or from Delegate method
  // OnWebResourceLoaderDone to safely delete this loader instance.
  void CloseAndDeleteSelf();

  // Initiate a download from the given URL.
  void LoadURL(const std::string& url);

  // Buffer to receive the resource data. If no buffer is provided, a 4K
  // internal buffer is used. Method set_content_buffer should be called
  // before starting a download or from within one of the delegate methods.
  void set_content_buffer(char* buffer, size_t size);
  const char* buffer() const { return buffer_; }
  int32_t data_size() const { return data_size_; }

  // Get info about the current download.
  const std::string url() const;
  int32_t GetHttpStatus() const;
  pp::URLResponseInfo GetResponseInfo() const;

  // Close the current connection. It is ok to use the same loader to download
  // from several URLs. However, the current connection must be closed before
  // starting a new one.
  void Close();

 private:
  // We use a single completion callback with a dispatch op-code to distinguish
  // between the various operations.
  void CompletionCallbackFunc(int32_t result, DispatchOpCode op_code);
  // Create a completion callback taking the given op-code.
  pp::CompletionCallback MakeCallback(DispatchOpCode op_code);

  // Helpers.
  void InitializeRequest(const std::string& url, pp::URLRequestInfo* request);
  void ReadNextDataBlock();
  void StartDownload(int32_t result, const std::string& url);

  // Main-thread callback for CloseAndDeleteSelf.
  static void DeleteOnMainThread(void* user_data, int32_t err);

  pp::CompletionCallbackFactory<WebResourceLoader,
                                threading::RefCount> factory_;
  pp::Instance* instance_;
  pp::URLLoader url_loader_;
  bool connected_;

  char* buffer_;
  size_t buffer_size_;
  int32_t data_size_;

  static const int32_t kInternalBufferSize = 4096;
  char internal_buffer_[kInternalBufferSize];

  Delegate* delegate_;
};


}  // namespace life

#endif  // WEB_RESOURCE_LOADER_H_
