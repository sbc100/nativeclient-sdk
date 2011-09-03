// Copyright 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "url_io/url_request.h"
#include <stdio.h>

namespace url_io {

URLRequest::URLRequest(std::string url)
  : url_(url),
    method_(kMethodGet),
    follow_redirect_(true),
    stream_to_file_(false),
    allow_cross_origin_requests_(false),
    allow_credentials_(false) {
}

URLRequest::URLRequest(std::string url, Method method)
  : url_(url),
    method_(method),
    follow_redirect_(true),
    stream_to_file_(false),
    allow_cross_origin_requests_(false),
    allow_credentials_(false) {
}

void URLRequest::SetHeader(std::string key, std::string value) {
  if (!key.empty()) {
    headers_[key] = value;
  }
}

std::string URLRequest::GetHeaderValueForKey(std::string key) {
  return headers_[key];
}

void URLRequest::RemoveHeader(std::string key) {
  HeaderDictionary::iterator i = headers_.find(key);
  if (i != headers_.end()) {
    headers_.erase(i);
  }
}

pp::URLRequestInfo URLRequest::GetRequestInfo(pp::Instance* instance) const {
  pp::URLRequestInfo request(instance);
  request.SetURL(url_);
  request.SetMethod((method_ == kMethodGet) ? "GET" : "POST");
  request.SetFollowRedirects(follow_redirect_);
  request.SetStreamToFile(stream_to_file_);
  request.SetAllowCrossOriginRequests(allow_cross_origin_requests_);
  request.SetAllowCredentials(allow_credentials_);

  HeaderDictionary::const_iterator i;
  std::string header_str;
  for (i = headers_.begin(); i != headers_.end(); ++i) {
    header_str = header_str + (*i).first + ": " + (*i).second + "\n";
  }
  request.SetHeaders(header_str);

  return request;
}

}  // namespace url_io
