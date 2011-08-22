// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "debugger/base/debug_socket.h"
#include <memory.h>
#include <string.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <errno.h>
#endif

namespace {
fd_set* kNoFdSet = NULL;
const int kMicroPerMilli = 1000;
const char* kAnyLocalHost = NULL;
sockaddr* kNoPeerAddress = NULL;
const int kWaitForOneWriteMs = 1000;
const int kWaitForOneReadMs = 1000;
const int kWriteBufferSize = 2048;
const int kLingerSeconds = 10;

bool InitSocketLib() {
#ifdef _WIN32
	WSADATA wsa_data;
  WORD version_requested = MAKEWORD(1, 1);
  return (WSAStartup(version_requested, &wsa_data) == 0);
#else
	return true;
#endif
}

void FreeSocketLib() {
#ifdef _WIN32
	WSACleanup();
#endif
}

timeval CreateTimeval(int milliseconds) {
  timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = milliseconds * kMicroPerMilli;
  return timeout;
}

sockaddr_in CreateSockAddr(const char* host, int port) {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);  // Convert port number from host byte order
                                // to network byte order.
  if ((NULL == host) || (strlen(host) == 0)) {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    hostent* hostDescr = gethostbyname(host);
    if (NULL != hostDescr)
      addr.sin_addr.s_addr =
        *(reinterpret_cast<unsigned int**>(hostDescr->h_addr_list)[0]);
  }
  return addr;
}

void SetSocketOptions(SOCKET sock) {
  if (INVALID_SOCKET != sock) {
    // Setup socket to flush pending data on close.
    linger ling;
    ling.l_onoff  = 1;
    // A socket should remain open for kLingerSeconds seconds after
    // a closesocket function call to enable queued data to be sent.
    ling.l_linger = kLingerSeconds;
    setsockopt(sock,
               SOL_SOCKET,
               SO_LINGER,
               reinterpret_cast<char*>(&ling),
               sizeof(ling));
    // Turn off buffering, to speedup debugger communication.
    int opt = 1;
    setsockopt(sock,
               IPPROTO_TCP,
               TCP_NODELAY,
               reinterpret_cast<char*>(&opt),
               sizeof(opt));
  }
}
}  // namespace

namespace debug {
SocketBase::SocketBase()
  : sock_(INVALID_SOCKET),
    init_success_(false),
    saved_last_error_(kSocketNoError) {
  init_success_ = InitSocketLib();
}

SocketBase::~SocketBase() {
  if (init_success_)
    FreeSocketLib();
}

int SocketBase::GetLastError() {
#ifdef _WIN32
	int res = WSAGetLastError();
  if (kSocketNoError == res)
    return saved_last_error_;
  return res;
#else
	return errno;
#endif
}

void SocketBase::Close() {
  if (INVALID_SOCKET != sock_) {
    saved_last_error_ = GetLastError();
#ifdef _WIN32
		closesocket(sock_);
#else
		shutdown(sock_, SHUT_RDWR);
		close(sock_);
#endif
    sock_ = INVALID_SOCKET;
  }
}

void SocketBase::ClearSavedLastError() {
  saved_last_error_ = kSocketNoError;
}

bool ListeningSocket::Listen(int port) {
  ClearSavedLastError();
  Close();
  sock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == sock_)
    return false;

  // Associate local address with socket.
  sockaddr_in addr = CreateSockAddr(kAnyLocalHost, port);
  if (bind(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
    Close();
    return false;
  }
  // Mark a socket as accepting connections.
  if (listen(sock_, SOMAXCONN) != 0) {
    Close();
  }
  return (INVALID_SOCKET != sock_);
}

bool ListeningSocket::Accept(int wait_ms, Socket* new_connection) {
  ClearSavedLastError();
  fd_set socks;
  FD_ZERO(&socks);
  FD_SET(sock_, &socks);

  // Wait for incoming connection.
  timeval timeout = CreateTimeval(wait_ms);
  if (select(sock_ + 1, &socks, kNoFdSet, kNoFdSet, &timeout) < 0) {
    Close();
    return false;
  }
  // No connection requests.
  if (!FD_ISSET(sock_, &socks))
    return false;

  // Accept a connection request.
  SOCKET sock = accept(sock_, kNoPeerAddress, 0);
  if (INVALID_SOCKET != sock)
    new_connection->AttachTo(sock);

  return new_connection->IsConnected();
}

void Socket::AttachTo(SOCKET sock) {
  Close();
  ClearSavedLastError();
  sock_ = sock;
  SetSocketOptions(sock_);
}

bool Socket::ConnectTo(const std::string& host, int port) {
  Close();
  ClearSavedLastError();
  sock_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == sock_)
    return false;

  sockaddr_in addr = CreateSockAddr(host.c_str(), port);
  if (connect(sock_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    Close();
  else
    SetSocketOptions(sock_);
  return IsConnected();
}

bool Socket::IsConnected() const {
  bool connected = (INVALID_SOCKET != sock_);
  if (!connected && (kSocketNoError == saved_last_error_))
		const_cast<Socket*>(this)->saved_last_error_ = 666;
  return connected;
}

size_t Socket::Write(const void* buff, size_t buff_len, int wait_ms) {
  if (!IsConnected())
    return 0;

  ClearSavedLastError();
  fd_set  socks;
  FD_ZERO(&socks);
  FD_SET(sock_, &socks);

  // Wait for 'write ready'.
  timeval timeout = CreateTimeval(wait_ms);
  if (select(sock_ + 1, kNoFdSet, &socks, kNoFdSet, &timeout) < 0) {
    Close();
    return 0;
  }
  int bytes_send = 0;
  if (FD_ISSET(sock_, &socks)) {
    bytes_send = send(sock_, static_cast<const char*>(buff), buff_len, 0);
    if (bytes_send < 0) {
      Close();
      return 0;
    }
  }
  return bytes_send;
}

// Blocks until all data has been sent.
size_t Socket::WriteAll(const void* buff, size_t buff_len) {
  const char* ptr = static_cast<const char*>(buff);
  size_t sent_bytes = 0;
  while (IsConnected() && (buff_len > 0)) {
    size_t wr = Write(ptr, buff_len, kWaitForOneWriteMs);
    sent_bytes += wr;
    buff_len -= wr;
    ptr += wr;
  }
  return sent_bytes;
}

size_t  Socket::WriteAll(const Blob& blob) {
  size_t sent_bytes = 0;
  while (IsConnected() && (sent_bytes < blob.size())) {
    char buff[kWriteBufferSize];
    size_t bytes_to_send = blob.Peek(sent_bytes, buff, sizeof(buff));
    sent_bytes += WriteAll(buff, bytes_to_send);
  }
  return sent_bytes;
}

size_t Socket::Read(void* buff, size_t buff_len, int wait_ms) {
  if (!IsConnected())
    return 0;

  ClearSavedLastError();
  fd_set socks;
  FD_ZERO(&socks);
  FD_SET(sock_, &socks);
  timeval timeout = CreateTimeval(wait_ms);

  // Wait for data.
  if (select(sock_ + 1, &socks, kNoFdSet, kNoFdSet, &timeout) < 0) {
    Close();
    return 0;
  }
  // No data available.
  if (!FD_ISSET(sock_, &socks))
    return 0;

	ssize_t read_bytes = recv(sock_, static_cast<char*>(buff), buff_len, 0);
	if ((-1 == read_bytes) || (0 == read_bytes)) {
    Close();
    return 0;
  }
  return read_bytes;
}

size_t Socket::ReadAll(void* buff, size_t buff_len) {
  size_t total_rd_bytes = 0;
  char* ptr = static_cast<char*>(buff);
  char* buff_end = ptr + buff_len;
  while (IsConnected() && (ptr < buff_end)) {
    size_t rd = Read(ptr, buff_len - total_rd_bytes, kWaitForOneReadMs);
    total_rd_bytes += rd;
    ptr += rd;
  }
  return total_rd_bytes;
}

}  // namespace debug

