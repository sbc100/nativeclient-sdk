// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXPERIMENTAL_VISUAL_STUDIO_PLUGIN_SRC_RSP_PROXY_PROXY_SOCKET_H_
#define EXPERIMENTAL_VISUAL_STUDIO_PLUGIN_SRC_RSP_PROXY_PROXY_SOCKET_H_

#include <winsock.h>
#include <string>

namespace debug {

// Implements a listening socket.
// Example:
// ListeningSocket lsn;
// lsn.Setup(4014);
// Socket conn;
// if (lsn.Accept(&conn, 200))
//   DoSomethingWithNewConnection(conn);

class Socket;
class ListeningSocket {
 public:
  ListeningSocket();
  ~ListeningSocket();

  bool Setup(int port);  // Listens on port.
  bool Accept(Socket* new_connection, int wait_ms);
  void Close();

 private:
  SOCKET sock_;
  bool init_success_;
};

// Implements a raw socket interface.
// Example:
// Socket conn;
// if (conn.ConnectTo("172.29.20.175", 4016))
//   DoSomethingWithNewConnection(conn);

class Socket {
 public:
  Socket();
  ~Socket();

  bool ConnectTo(const std::string& host, int port);
  bool IsConnected() const;
  void Close();
  size_t Read(void* buff, size_t sz, int wait_ms);
  size_t Write(const void* buff, size_t sz, int wait_ms);
  void WriteAll(const void* buff, size_t sz);

 private:
  void AttachTo(SOCKET sock);

  SOCKET sock_;
  bool init_success_;
  friend class ListeningSocket;
};
}  // namespace debug
#endif  // EXPERIMENTAL_VISUAL_STUDIO_PLUGIN_SRC_RSP_PROXY_PROXY_SOCKET_H_


