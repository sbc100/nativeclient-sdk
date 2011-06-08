// Copyright 2011 The Native Client SDK Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#ifndef SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_DEBUG_SOCKET_H_
#define SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_DEBUG_SOCKET_H_

#include <winsock.h>
#include <string>
#include "debugger/base/debug_blob.h"

namespace debug {
class Socket;

/// Implements a listening TCP/IP socket.
///
/// Example:
/// ListeningSocket lsn;
/// lsn.Listen(4014);
/// Socket conn;
/// int wait_ms = 100;
/// if (lsn.Accept(wait_ms, &conn))
///   DoSomethingWithNewConnection(conn);
///
/// Note: add wsock32.lib to the list of linked libraries.
class ListeningSocket {
 public:
  ListeningSocket();
  ~ListeningSocket();

  /// Starts listening for incoming connection.
  /// @param[in] port port to listen on.
  /// @return true if operation succeeds.
  bool Listen(int port);

  /// Receives incoming connection, if any.
  /// @param[in] wait_ms number of milliseconds to wait for connection
  /// @param[out] new_connection pointer to |Socket| object that
  /// receives connection
  /// @return true if connection is received.
  bool Accept(int wait_ms, Socket* new_connection);

  /// Destroys listening socket.
  void Close();

 private:
  SOCKET sock_;
  bool init_success_;
};

/// Implements a raw socket interface.
///
/// Example:
/// Socket conn;
/// if (conn.ConnectTo("172.29.20.175", 4016))
///   DoSomethingWithNewConnection(conn);
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
  void WriteAll(const Blob& blob);

 private:
  void AttachTo(SOCKET sock);

  SOCKET sock_;
  bool init_success_;
  friend class ListeningSocket;
};
}  // namespace debug
#endif  // SRC_EXPERIMENTAL_DEBUG_SERVER_COMMON_DEBUG_SOCKET_H_


