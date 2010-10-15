// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef EXAMPLES_GETURL_GETURL_HANDLER_H_
#define EXAMPLES_GETURL_GETURL_HANDLER_H_

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/dev/url_loader_dev.h>
#include <ppapi/cpp/dev/url_request_info_dev.h>
#include <ppapi/cpp/instance.h>
#include <string>

// MethodCallback<T> may be used to create CompletionCallback
// objects that are bound to member functions.
//
// EXAMPLE USAGE:
//
//   class MyHandler {
//    public:
//     MyHandler() : read_callback_(this) {...}
//     void Read() {
//       url_loader_.ReadResponseBody(buff_,sizeof(buff_), read_callback_);
//     }
//     void OnRead(int32_t result) {
//       ...
//     }
//   MethodCallback<MyHandler, &MyHandler::OnRead> read_callback_;
//   pp::URLLoader_Dev url_loader_;
//   char buff_[100];
//   }
//
template <typename T, void (T::*Method)(int32_t result)>
class MethodCallback : public pp::CompletionCallback {
 public:
  explicit MethodCallback(T* object)
      : pp::CompletionCallback(CompletionCallbackFunc, object) {}
  static void CompletionCallbackFunc(void* user_data, int32_t result) {
    (reinterpret_cast<T*>(user_data)->*Method)(result);
  }
};

// GetURLHandler is used to download data from |url|. When download is
// finished or when an error occurs, it calls JavaScript function
// reportResult(url, result, success) (defined in geturl.html) and
// self-destroys.
//
// EXAMPLE USAGE:
// GetURLHandler* handler* = GetURLHandler::Create(instance,url);
// if (!handler->Start())
//   delete handler;
//
//
class GetURLHandler {
 public:
  ~GetURLHandler();
  // Creates instance of GetURLHandler on the heap.
  // GetURLHandler objects shall be created only on the heap (they
  // self-destroy when all data is in).
  static GetURLHandler* Create(const pp::Instance& instance_,
                               const std::string& url);
  // Initiates page (URL) download.
  // Returns false in case of internal error, user is responsible for
  // deleting GetURLHandler object. If true is returne, object will
  // self-destroy when download is finished.
  bool Start();

 private:
  static const int kBufferSize = 4096;

  GetURLHandler(const pp::Instance& instance_, const std::string& url);

  // Callback fo the pp::URLLoader::Open().
  // Called by pp::URLLoader when response headers are received or when an
  // error occurs (in response to the call of pp::URLLoader::Open()).
  // Look at <ppapi/c/dev/ppb_url_loader_dev.h> and
  // <ppapi/cpp/dev/url_loader_dev.h> for more information about pp::URLLoader.
  void OnOpen(int32_t result);

  // Callback fo the pp::URLLoader::ReadResponseBody().
  // |result| contains the number of bytes read or an error code.
  // Appends data from this->buffer_ to this->url_response_body_.
  void OnRead(int32_t result);

  // Reads the response body (asynchronously) into this->buffer_.
  // OnRead() will be called when bytes are received or when an error occurs.
  void ReadBody();

  // Calls JS reportResult() defined in geturl.html
  void ReportResult(const std::string& fname,
                    const std::string& text,
                    bool success);
  // Calls JS reportResult() and self-destroy (aka delete this;).
  void ReportResultAndDie(const std::string& fname,
                          const std::string& text,
                          bool success);

  // CompletionCallback objects that are bound to member functions.
  MethodCallback<GetURLHandler, &GetURLHandler::OnOpen> open_callback_;
  MethodCallback<GetURLHandler, &GetURLHandler::OnRead> read_callback_;

  PP_Instance instance_id_;
  std::string url_;  // URL to be downloaded.
  pp::URLRequestInfo_Dev url_request_;
  pp::URLLoader_Dev url_loader_;  // URLLoader provides an API to download URLs.
  char buffer_[kBufferSize];  // buffer for pp::URLLoader::ReadResponseBody().
  std::string url_response_body_;  // Contains downloaded data.

  GetURLHandler(const GetURLHandler&);
  void operator=(const GetURLHandler&);
};

#endif  // EXAMPLES_GETURL_GETURL_HANDLER_H_

