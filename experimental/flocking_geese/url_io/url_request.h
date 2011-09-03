// Copyright 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef URL_REQUEST_H_
#define URL_REQUEST_H_

#include <map>
#include <ppapi/cpp/url_request_info.h>

namespace pp {
class Instance;
}  // namespace pp

namespace url_io {

///
class URLRequest {
 public:
  enum Method { kMethodGet, kMethodPost };

  explicit URLRequest(std::string url);
  URLRequest(std::string url, Method method);

  /// Set the URL for the request.
  /// @param url the url string.
  void set_url(std::string url) { url_ = url; }
  /// Query the current URL for this request.
  /// @return the URL string.
  std::string url() const {return url_; }

  /// Set the follow-redirect property. When set to true Chrome/NaCl
  /// automatically follow redirect requests. When false, it is the users
  /// responsibility to catch redirect headers and follow them. (@see
  /// WebResourceLoader::FollowRedirect.
  /// @param follow_redirect new boolean value for follow-redirect property.
  void set_follow_redirect(bool follow_redirect) {
    follow_redirect_ = follow_redirect;
  }
  /// Query the current value for follow-redirect property.
  /// @return true/false property for property follow-redirect.
  bool follow_redirect() const { return follow_redirect_; }

  /// Set the stream-to-file property.
  /// @param stream_to_file new boolean value for stream-to-file property.
  void set_stream_to_file(bool stream_to_file) {
    stream_to_file_ = stream_to_file;
  }
  /// Query the current value for property stream-to-file.
  /// @return true/false boolean value for property stream-to-file.
  bool stream_to_file() const { return stream_to_file_; }

  /// Set the allow-cross-origin-requests property.
  /// @param allow_cross_origin_requests new boolean value for allow-cross-
  /// origin-requests property.
  void set_allow_cross_origin_requests(bool allow_cross_origin_requests) {
    allow_cross_origin_requests_ = allow_cross_origin_requests;
  }
  /// @return boolean value for property allow-cross-oriogin-requests.
  bool allow_cross_origin_requests() const {
    return allow_cross_origin_requests_;
  }

  /// Set the allow-credentials property.
  /// @param allow_credentials new boolean value for allow-credentials property.
  void set_allow_credentials(bool allow_credentials) {
    allow_credentials_ = allow_credentials;
  }
  /// @return boolean value for property allow-credentials.
  bool allow_credentials() const { return allow_credentials_; }

  /// Set a request header. For instance, SetHeader("Accept", "text/*") causes
  /// header "Accept: text/*" to be included in the http request. It is ok to
  /// set a header value to an empty string. However, such headers are not
  /// included in the http request.
  /// @param key field name for the header, e.g. "Accept."
  /// @param value field value for the header, e.g. "text/*."
  void SetHeader(std::string key, std::string value);
  /// Query the field value for a given header name.
  /// @return field value for header with name key. Return an empty string if
  /// there is no such header in the dictionary.
  std::string GetHeaderValueForKey(std::string key);
  /// Remove the header with field name key.
  /// @param key name of field to remove.
  void RemoveHeader(std::string key);

  /// Produce a URLRequestInfo instance suitable for use with pp::URLLoader.
  /// @param instance the pp::Instance with which the request will be used.
  /// @return a pp::URLRequestInfo instance.
  pp::URLRequestInfo GetRequestInfo(pp::Instance* instance) const;

  // TODO(gwink): add support for (1) body data, (2) custom referrer,
  //     (3) download progress, (4) upload progress, (5) transfer encoding,
  //     (6) buffer thresholds, (7) cross-origin requests.

 private:
  typedef std::map<std::string, std::string> HeaderDictionary;

  std::string url_;
  Method method_;
  HeaderDictionary headers_;
  bool follow_redirect_;
  bool stream_to_file_;
  bool allow_cross_origin_requests_;
  bool allow_credentials_;
};

}  // namespace url_io
#endif  // URL_REQUEST_H_
