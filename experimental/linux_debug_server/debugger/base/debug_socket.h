// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEBUGGER_BASE_DEBUG_SOCKET_H_
#define DEBUGGER_BASE_DEBUG_SOCKET_H_

#ifdef _WIN32
#include <winsock.h>
#else
#define SOCKET int
#define INVALID_SOCKET -1
#endif

#include <string>
#include "debugger/base/debug_blob.h"

namespace debug {
class Socket;

class SocketBase {
 public:
  static const int kSocketNoError = 0;

  SocketBase();
  virtual ~SocketBase();

  /// Destroys socket. Further attempt to use it will return false.
  /// Close() can be called many times, it has no effect on a closed
  /// connection.  You can safely reuse object after it has been closed.
  void Close();

  /// @return returns the error status for the last sockets operation that
  /// failed for the calling thread
  /// Look here for erroc codes description:
  /// http://msdn.microsoft.com/en-us/library/ms740668%28v=vs.85%29.aspx
  int GetLastError();

 protected:
  // DISALLOW_COPY_AND_ASSIGN
  SocketBase(const SocketBase&);
  SocketBase& operator = (const SocketBase&);
  void ClearSavedLastError();

  SOCKET sock_;
  bool init_success_;
  int saved_last_error_;
};

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
class ListeningSocket : public SocketBase {
 public:
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
};

/// Implements a raw socket interface.
///
/// Example:
/// Socket conn;
/// if (conn.ConnectTo("172.29.20.175", 4016))
///   DoSomethingWithNewConnection(conn);
class Socket : public SocketBase {
 public:
  /// Attempts to establish connection to |host| on |port|.
  /// @param[in] host name of destinamtion host.
  /// @param[in] port TCP/IP port number to connect to
  /// @return true if connection is established.
  bool ConnectTo(const std::string& host, int port);

  /// @return true if connection is alive.
  bool IsConnected() const;

  /// Reads data from connection.
  /// @param[out] buff buffer for incoming data
  /// @param[in] buff_len size of the |buff| in bytes
  /// @param[in] wait_ms number of milliseconds to wait
  /// @return number of received bytes
  /// Note: function blocks for not more then |wait_ms| milliseconds.
  size_t Read(void* buff, size_t buff_len, int wait_ms);

  /// Writes (sends) data to the connection.
  /// @param[in] buff buffer with data to send.
  /// @param[in] buff_len size of the |buff| in bytes
  /// @param[in] wait_ms number of milliseconds to wait
  /// the number of bytes sent
  /// @return number of bytes sent
  /// Note: function blocks for not more then |wait_ms| milliseconds.
  size_t Write(const void* buff, size_t buff_len, int wait_ms);

  /// Writes (sends) data to the connection.
  /// Blocks until all data is gone or connection is closed.
  /// @param[in] buff buffer with data to send.
  /// @param[in] buff_len size of the |buff| in bytes
  /// @return number of written bytes
  size_t WriteAll(const void* buff, size_t buff_len);

  /// Writes (sends) data to the connection.
  /// Blocks until all data is gone or connection is closed.
  /// @param[in] blob data to send.
  /// @return number of written bytes
  size_t WriteAll(const Blob& blob);

  /// Reads data to the connection.
  /// Blocks until |buff_len| bytes received or connection is closed.
  /// @param[out] buff buffer for incoming data
  /// @param[in] buff_len size of the |buff| in bytes
  /// @return number of written bytes
  size_t ReadAll(void* buff, size_t buff_len);

 private:
  void AttachTo(SOCKET sock);

  friend class ListeningSocket;
};
}  // namespace debug
#endif  // DEBUGGER_BASE_DEBUG_SOCKET_H_


