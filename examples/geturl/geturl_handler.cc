// Copyright 2010 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include "examples/geturl/geturl_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <ppapi/c/pp_errors.h>
#include <ppapi/c/ppb_instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>

namespace {
bool IsError(int32_t result) {
  return ((PP_OK != result) && (PP_ERROR_WOULDBLOCK != result));
}
}  // namespace

GetURLHandler::~GetURLHandler() {
}

GetURLHandler* GetURLHandler::Create(const pp::Instance& instance,
                                     const std::string& url) {
  return new GetURLHandler(instance, url);
}

GetURLHandler::GetURLHandler(const pp::Instance& instance,
                             const std::string& url)
    : open_callback_(this),
      read_callback_(this),
      instance_id_(instance.pp_instance()),
      url_(url),
      url_loader_(instance) {
  url_request_.SetURL(url);
  url_request_.SetMethod("GET");
}

bool GetURLHandler::Start() {
  int32_t res = url_loader_.Open(url_request_, open_callback_);
  if (IsError(res)) {
    ReportResult(url_, "pp::URLLoader_Dev::Open() failed", false);
    return false;
  }
  return true;
}

void GetURLHandler::OnOpen(int32_t result) {
  if (result < 0)
    ReportResultAndDie(url_, "pp::URLLoader_Dev::Open() failed", false);
  else
    ReadBody();
}

void GetURLHandler::OnRead(int32_t result) {
  if (result < 0) {
    ReportResultAndDie(url_,
                       "pp::URLLoader_Dev::ReadResponseBody() result<0",
                       false);

  } else if (result != 0) {
    int32_t num_bytes = result < kBufferSize ? result : sizeof(buffer_);
    for (int32_t i = 0; i < num_bytes; i++)
      url_response_body_.push_back(buffer_[i]);
    ReadBody();
  } else {
    ReportResultAndDie(url_, url_response_body_, true);
  }
}

void GetURLHandler::ReadBody() {
  // Reads the response body (asynchronous) into this->buffer_.
  // OnRead() will be called when bytes are received or when an error occurs.
  // Look at <ppapi/c/dev/ppb_url_loader> for more details.
  int32_t res = url_loader_.ReadResponseBody(buffer_,
                                             sizeof(buffer_),
                                             read_callback_);
  if (IsError(res)) {
    ReportResultAndDie(url_,
                       "pp::URLLoader_Dev::ReadResponseBody() failed",
                       false);
  }
}

void GetURLHandler::ReportResultAndDie(const std::string& fname,
                                       const std::string& text,
                                       bool success) {
  ReportResult(fname, text, success);
  delete this;
}

void GetURLHandler::ReportResult(const std::string& fname,
                                 const std::string& text,
                                 bool success) {
  if (success)
    printf("GetURLHandler::ReportResult(Ok).\n");
  else
    printf("GetURLHandler::ReportResult(Err). %s\n", text.c_str());
  pp::Module* module = pp::Module::Get();
  if (NULL == module)
    return;

  pp::Instance* instance = module->InstanceForPPInstance(instance_id_);
  if (NULL == instance)
    return;

  pp::Var window = instance->GetWindowObject();
  // calls JavaScript function reportResult(url, result, success)
  // defined in geturl.html.
  pp::Var exception;
  window.Call("reportResult", fname, text, success, &exception);
}

