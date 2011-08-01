// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_RESOURCE_LOADER_H_
#define WEB_RESOURCE_LOADER_H_

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <map>
#include <string>

#include "c_salt/threading/pthread_ext.h"
#include "c_salt/threading/ref_count.h"

namespace pp {
class Instance;
class URLRequestInfo;
}  // namespace pp

namespace c_salt {

class URLRequest;

/// WebResourceLoader provides support for downloading resource data from a
/// given URL. It is usable from any thread and is thread safe. The only
/// retriction is that instance must be created on the main thread. To use
/// WebResourceLoader you must implement interface IDelegate and provide a
/// pointer to a delegate when creating the WebResourceLoader instance.
class WebResourceLoader {
 public:
  /// Interface IDelegate must be implemented by classes wishing to use
  /// WebResourceLoader.
  class Delegate {
   public:
    typedef WebResourceLoader Loader;

    virtual ~Delegate() = 0;

    /// Called when the URLResponseInfo has been received. You must call
    /// WebResourceLoader::ReadMoreData after receiving the response info to
    /// start downloading the resource. It is ok to call ReadMoreData from
    /// within OnLoaderReceivedResponseInfo.
    /// @param[in] loader The WebResourceLoader instance invoking this function.
    virtual void OnLoaderReceivedResponseInfo(Loader* loader) = 0;

    /// Called when a block of web-resource data has been received.
    /// @param[in] loader The WebResourceLoader instance invoking this function.
    virtual void OnLoaderReceivedData(Loader* loader) = 0;

    /// Called after all the web-resource data has been received.
    /// @param[in] loader The WebResourceLoader instance invoking this function.
    virtual void OnLoaderCompletedDownload(Loader* loader) = 0;

    /// Called when an error occurs.
    /// @param[in] error PPAPI error code (see pp_errors.h).
    /// @param[in] loader The WebResourceLoader instance invoking this function.
    virtual void OnLoaderError(int32_t error, Loader* loader) = 0;

    /// Called when the loader is done downloading data and is ready to be
    /// closed, either at the end of a successful download or following an
    /// error. If you plan on reusing the loader, you can simply close it with
    /// WebResourceLoader::Close. If you do not need it any more, you can get
    /// rid of it with WebResourceLoader::CloseAndDeleteSelf. Either of these
    /// functions can be called from within OnLoaderDone.
    /// @param[in] loader The WebResourceLoader instance invoking this function.
    virtual void OnLoaderDone(Loader* loader) = 0;
  };

  /// A dictionary of <key, value> pairs for response-info http headers.
  typedef std::map<std::string, std::string> HeaderDictionary;

  WebResourceLoader(pp::Instance* instance, Delegate* delegate);

  /// Call this function from any thread or from within an IDelegate function
  /// to close and delete the WebResourceLoader instance.
  void CloseAndDeleteSelf();

  /// Initiate a download from the given URL. LoadURL will fail, with an assert
  /// in debug mode, if a download is already in progress.
  /// @param[in] url_request request configured with the desired download
  ///     parameters and URL.
  void LoadURL(const URLRequest& url_request);

  /// WebResourceLoader doesn't automaticaly keep downloading data. Instead,
  /// you must continually call ReadMoreData to get the next chunk of data until
  /// WebResourceLoader calls Delegate function OnLoaderDownloadComplete.
  /// @return true if the next download is successfully scheduled.
  bool ReadMoreData();

  /// Set the memory buffer to receive the next chunk of data. The size of the
  /// buffer determines how much data WebResourceLoader will try to get before
  /// calling Delegate::OnLoaderDataReceived. It is ok to set a new buffer for
  /// each data-chunk download. It is also ok to call this function from within
  /// Delegate::OnLoaderDataReceived. If no buffer is provided, a 4K internal
  /// buffer is used.
  /// @param[in] buffer pointer to memory buffer to receive the next chunk of
  ///     data.
  /// @param[in] size size of buffer in bytes.
  void set_content_buffer(uint8_t* buffer, int32_t size);
  /// Get the current download buffer. Note: Do not attempt to read the buffer
  /// while a ReadMoreData asynchronous call is in progress. Instead, wait until
  /// the delegate is invoked.
  /// @return pointer to the current download buffer.
  const uint8_t* buffer() const { return buffer_; }
  /// Get the size of the current download buffer. Note: Do not attempt to read
  /// the data size while a ReadMoreData asynchronous call is in progress.
  /// Instead, wait until the delegate is invoked.
  /// @return size in bytes.
  int32_t data_size() const { return data_size_; }

  /// Get the URL for the current download. This function should only be called
  /// from the main thread.
  /// @return URL string for the current download.
  const std::string url() const;
  /// Get the http status for the current download. This function should only
  /// be called from the main thread.
  /// @return http status code.
  int32_t GetHttpStatus() const;
  /// Get the URLResponseInfo object for the current download. Returns a
  /// null object if the response has not yet been received. This function
  /// should only be called from the main thread.
  /// @return the URLResponseInfo object.
  pp::URLResponseInfo GetResponseInfo() const;
  /// Get the length of the content, as reported by the server, extracted from
  /// the content-length parameter in the response info.
  /// @return the content length in bytes.
  int32_t GetContentLength() const;

  /// Return a dictionary of <key, value> pairs for the response headers.
  /// Keys are in all-upper case.
  /// @return dictionary of <key, value> header pairs.
  const HeaderDictionary& response_headers() const { return response_headers_; }

  /// Close the current connection. It is ok to use the same loader to download
  /// from several URLs. However, the current connection must be closed before
  /// starting a new one. Close may only be called from the main thread.
  void Close();

 protected:
    // Do not delete a WebResourceLoader instance directly. Call
    // Call CloseAndDeleteSelf instead.
    virtual ~WebResourceLoader();

 private:
  // WebResourceLoader uses a state machine to keep track of its internal state.
  enum State {
    kStateNotConnected = 0,  // No active connection.
    kStateConnecting,  // Establishing a connection, waiting for response info.
    kStateSuspended,  // Suspended until ReadNextBlock is called.
    kStateInvokedDelegate,  // In a delegate-interface function.
    kStateWillReadMoreData,  // Read next block when returning from delegate.
    kStateDownloading  // Currently waiting for an asynch. data-read callback.
  };

  // We use a single completion callback with a dispatch op-code to distinguish
  // between the various asynchronous operations.
  enum CallbackOpCode {
    kResponseInfoCallback,
    kDataReceivedCallback
  };

  void CompletionCallbackFunc(int32_t result, CallbackOpCode op_code);

  void DoResponseInfoReceived(int32_t result);
  void DoDataReceived(int32_t result);
  void DoDownloadError(int32_t result);

  // Create a completion callback taking the given op-code.
  pp::CompletionCallback MakeCallback(CallbackOpCode op_code);

  // Parse response headers into the header dictionary.
  void ParseHeaders(pp::URLResponseInfo response);

  // Main-thread-only functions, suitable for callbacks.
  void InternalReadMoreData(int32_t result);
  void InternalLoadURL(int32_t result, const URLRequest& url_request);

  // Main-thread callback for CloseAndDeleteSelf.
  static void DeleteOnMainThread(void* user_data, int32_t err);

  pp::CompletionCallbackFactory<WebResourceLoader,
                                ::threading::RefCount> factory_;
  pp::Instance* instance_;
  pp::URLLoader url_loader_;

  State state_;
  mutable pthread_mutex_t state_mutex_;

  HeaderDictionary response_headers_;

  // Download buffer, either user provided, or as a pointer to internal_buffer_
  // otherwise.
  uint8_t* buffer_;
  int32_t buffer_size_;
  int32_t data_size_;

  static const int32_t kInternalBufferSize = 4096;
  uint8_t internal_buffer_[kInternalBufferSize];

  Delegate* delegate_;
};

inline WebResourceLoader::Delegate::~Delegate() {}

}  // namespace c_salt

#endif  // WEB_RESOURCE_LOADER_H_
