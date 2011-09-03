// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "url_io/web_resource_loader.h"

#include <algorithm>
#include <cassert>
#include <stdio.h>
#include "ppapi/c/pp_errors.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/url_loader.h"
#include "ppapi/cpp/url_request_info.h"
#include "ppapi/cpp/url_response_info.h"

#include "threading/scoped_mutex_lock.h"
#include "url_io/url_request.h"

namespace {
// Skip white space in string str, starting at index pos. Returns the index of
// the first non white-space character found, or str.length() if only
// whitespace characters are found.
size_t SkipWhiteSpace(const std::string& str, size_t pos) {
  size_t str_length = str.length();
  while (pos < str_length && ::isspace(str[pos])) {
    ++pos;
  }
  return pos;
}
}  // anonymous namespace

namespace url_io {

WebResourceLoader::WebResourceLoader(pp::Instance* instance,
                                     WebResourceLoader::Delegate* delegate)
  : factory_(this),
    instance_(instance),
    url_loader_(*instance),
    state_(kStateNotConnected),
    buffer_(NULL),
    buffer_size_(0),
    data_size_(0),
    delegate_(delegate) {
  assert(delegate != NULL);
  pthread_mutex_init(&state_mutex_, NULL);
}

void WebResourceLoader::CloseAndDeleteSelf() {
  // Can't close & delete this instance here. Instead, we schedule a main-thread
  // callback to delete it later.
  pp::CompletionCallback cc(DeleteOnMainThread, this);
  pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
}

WebResourceLoader::~WebResourceLoader() {
  Close();
  pthread_mutex_destroy(&state_mutex_);
}

void WebResourceLoader::LoadURL(const URLRequest& url_request) {
  // TODO(gwink): add support for stream-to-file.
  assert(!url_request.stream_to_file() && "Stream-to-file not implemented.");
  // Check that there is no pending request.
  assert(state_ == kStateNotConnected);

  if (state_ == kStateNotConnected) {
    state_ = kStateConnecting;
    pp::CompletionCallback cc = factory_.NewCallback(
        &WebResourceLoader::InternalLoadURL, url_request);
    pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
  }
}

bool WebResourceLoader::ReadMoreData() {
  pthread_mutex_lock(&state_mutex_);
  assert(state_ == kStateInvokedDelegate || state_ == kStateSuspended);

  if (state_ == kStateSuspended) {
    // Call resume on the main thread.
    state_ = kStateDownloading;
    pthread_mutex_unlock(&state_mutex_);
    pp::CompletionCallback cc = factory_.NewCallback(
        &WebResourceLoader::InternalReadMoreData);
    pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
    return true;
  } else if (state_ == kStateInvokedDelegate) {
    // We're currently within a delegate function. Advance the state to
    // will-read-more-data; that insures we'll schedule the next read op when
    // the delegate returns control to this instance.
    state_ = kStateWillReadMoreData;
    pthread_mutex_unlock(&state_mutex_);
    return true;
  }
  state_ = kStateNotConnected;
  pthread_mutex_unlock(&state_mutex_);
  return false;
}

void WebResourceLoader::set_content_buffer(uint8_t* buffer, int32_t size) {
  buffer_ = buffer;
  buffer_size_ = size;
}

const std::string WebResourceLoader::url() const {
  pp::URLResponseInfo response = url_loader_.GetResponseInfo();
  return response.GetURL().AsString();
}

int32_t WebResourceLoader::GetHttpStatus() const {
  return GetResponseInfo().GetStatusCode();
}

pp::URLResponseInfo WebResourceLoader::GetResponseInfo() const {
  assert(pp::Module::Get()->core()->IsMainThread());
  pp::URLResponseInfo response = url_loader_.GetResponseInfo();
  assert(!response.is_null());
  return response;
}

int32_t WebResourceLoader::GetContentLength() const {
  HeaderDictionary::const_iterator it =
      response_headers_.find("CONTENT-LENGTH");
  if (it == response_headers_.end()) {
    return 0;
  } else {
    // Return content-length value string, converted to int.
    return static_cast<int32_t>(atol((*it).second.c_str()));
  }
}

void WebResourceLoader::Close() {
  assert(pp::Module::Get()->core()->IsMainThread());
  url_loader_.Close();
  buffer_ = NULL;
  buffer_size_ = 0;
  data_size_ = 0;
  state_ = kStateNotConnected;
  response_headers_.clear();
}

void WebResourceLoader::DoResponseInfoReceived(int32_t result) {
  pthread_mutex_lock(&state_mutex_);
  pp::URLResponseInfo response = url_loader_.GetResponseInfo();
  ParseHeaders(response);
  state_ = kStateInvokedDelegate;
  pthread_mutex_unlock(&state_mutex_);
  delegate_->OnLoaderReceivedResponseInfo(this);

  pthread_mutex_lock(&state_mutex_);
  if (state_ == kStateWillReadMoreData) {
    state_ = kStateDownloading;
    pthread_mutex_unlock(&state_mutex_);
    pp::CompletionCallback cc = factory_.NewCallback(
        &WebResourceLoader::InternalReadMoreData);
    pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
  } else {
    state_ = kStateSuspended;
    pthread_mutex_unlock(&state_mutex_);
  }
}

void WebResourceLoader::DoDataReceived(int32_t result) {
  pthread_mutex_lock(&state_mutex_);
  if (result == 0) {
    // No more data; the download completed successfully.
    state_ = kStateNotConnected;
    data_size_ = 0;
    pthread_mutex_unlock(&state_mutex_);
    delegate_->OnLoaderCompletedDownload(this);
    delegate_->OnLoaderDone(this);
  } else {
    assert(result > 0);
    // We received a block of data of size |result|.
    data_size_ = result;
    state_ = kStateInvokedDelegate;
    pthread_mutex_unlock(&state_mutex_);
    delegate_->OnLoaderReceivedData(this);

    pthread_mutex_lock(&state_mutex_);
    if (state_ == kStateWillReadMoreData) {
      state_ = kStateDownloading;
      pthread_mutex_unlock(&state_mutex_);
      pp::CompletionCallback cc = factory_.NewCallback(
          &WebResourceLoader::InternalReadMoreData);
      pp::Module::Get()->core()->CallOnMainThread(0, cc, PP_OK);
    } else {
      state_ = kStateSuspended;
      pthread_mutex_unlock(&state_mutex_);
    }
  }
}

void WebResourceLoader::DoDownloadError(int32_t result) {
  pthread_mutex_lock(&state_mutex_);
  state_ = kStateNotConnected;
  pthread_mutex_unlock(&state_mutex_);
  delegate_->OnLoaderError(result, this);
  delegate_->OnLoaderDone(this);
}

void WebResourceLoader::CompletionCallbackFunc(int32_t result,
                                               CallbackOpCode op_code) {
  // TODO(gwink): add support for follow-redirect.
  if (result < 0) {
    // A negative value indicates an error.
    DoDownloadError(result);
  } else if (result == PP_OK && state_ == kStateConnecting) {
  // If we're not yet connected and result == PP_OK, then we received
  // the http headers.
    assert(op_code == kResponseInfoCallback);
    DoResponseInfoReceived(result);
  } else {
    assert(op_code == kDataReceivedCallback);
    DoDataReceived(result);
  }
}

pp::CompletionCallback WebResourceLoader::MakeCallback(CallbackOpCode op_code) {
  return factory_.NewCallback(
      &WebResourceLoader::CompletionCallbackFunc, op_code);
}

void WebResourceLoader::InternalLoadURL(int32_t result,
                                        const URLRequest& url_request) {
  if (result == PP_OK) {
    // Only usable from main plugin thread.
    assert(pp::Module::Get()->core()->IsMainThread());

    pp::CompletionCallback cc = MakeCallback(kResponseInfoCallback);
    int32_t rv = url_loader_.Open(url_request.GetRequestInfo(instance_), cc);
    assert(rv == PP_OK_COMPLETIONPENDING);
  }
}

void WebResourceLoader::InternalReadMoreData(int32_t result) {
  if (result == PP_OK) {
    // If a custom buffer has not been set or has been set to NULL, use the
    // internal buffer.
    if (buffer_ == NULL) {
      buffer_ = internal_buffer_;
      buffer_size_ = kInternalBufferSize;
    }
    // Get the next block of data.
    pp::CompletionCallback cc = MakeCallback(kDataReceivedCallback);
    int32_t rv = url_loader_.ReadResponseBody(buffer_, buffer_size_, cc);
    assert(rv == PP_OK_COMPLETIONPENDING);
  }
}

void WebResourceLoader::ParseHeaders(pp::URLResponseInfo response) {
  // The headers dictionary should be empty.
  assert(response_headers_.empty());
  response_headers_.clear();

  // Get the headers as a string.
  pp::Var headers_var = response.GetHeaders();
  assert(headers_var.is_string());
  if (!headers_var.is_string())
    return;
  std::string headers = headers_var.AsString();

  // Parse headers.
  size_t i_line = 0;
  size_t headers_length = headers.length();
  std::string key;
  while (i_line != std::string::npos && i_line < headers_length) {
    // A line that starts with space or tab continues the previous line.
    size_t i_value = i_line;
    if (!::isspace(headers[i_line])) {
      // Extract the key.
      size_t i_key = i_line;
      size_t i_separator = headers.find(':', i_key);
      if (i_separator == std::string::npos)
        return;
      key = headers.substr(i_key, i_separator - i_key);
      i_value = i_separator + 1;
      // Convert key to all upper case.
      std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    }

    // Extract the value.
    i_value = SkipWhiteSpace(headers, i_value);
    if (i_value >= headers_length)
      return;
    size_t i_eol = headers.find_first_of("\r\n", i_value);
    std::string value = headers.substr(
        i_value, (i_eol == std::string::npos)? i_eol : i_eol - i_value);


    if (!value.empty() && !key.empty()) {
      // Append the value to what's already in the map; that takes care of
      // values that are spread over multiple lines.
      response_headers_[key].append(value);
    }

    // Skip crlf at eol.
    while (i_eol < headers_length &&
           (headers[i_eol] == '\r' || headers[i_eol] == '\n')) {
      ++i_eol;
    }
    i_line = i_eol;
  }
}

void WebResourceLoader::DeleteOnMainThread(void* user_data, int32_t err) {
  WebResourceLoader* loader = static_cast<WebResourceLoader*>(user_data);
  if (loader != NULL) {
    loader->Close();
    delete loader;
  }
}

}  // namespace url_io
