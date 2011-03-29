// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_GETURL_GETURL_HANDLER_H_
#define EXAMPLES_GETURL_GETURL_HANDLER_H_

#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/instance.h>
#include <string>

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
  // Creates instance of GetURLHandler on the heap.
  // GetURLHandler objects shall be created only on the heap (they
  // self-destroy when all data is in).
  static GetURLHandler* Create(pp::Instance* instance_,
                               const std::string& url);
  // Initiates page (URL) download.
  // Returns false in case of internal error, and self-destroys.
  bool Start();

 private:
  static const int kBufferSize = 4096;

  GetURLHandler(pp::Instance* instance_, const std::string& url);
  ~GetURLHandler();

  // Callback fo the pp::URLLoader::Open().
  // Called by pp::URLLoader when response headers are received or when an
  // error occurs (in response to the call of pp::URLLoader::Open()).
  // Look at <ppapi/c/ppb_url_loader.h> and
  // <ppapi/cpp/url_loader.h> for more information about pp::URLLoader.
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

  PP_Instance instance_id_;
  std::string url_;  // URL to be downloaded.
  pp::URLRequestInfo url_request_;
  pp::URLLoader url_loader_;  // URLLoader provides an API to download URLs.
  char buffer_[kBufferSize];  // buffer for pp::URLLoader::ReadResponseBody().
  std::string url_response_body_;  // Contains downloaded data.
  pp::CompletionCallbackFactory<GetURLHandler> cc_factory_;

  GetURLHandler(const GetURLHandler&);
  void operator=(const GetURLHandler&);
};

#endif  // EXAMPLES_GETURL_GETURL_HANDLER_H_

