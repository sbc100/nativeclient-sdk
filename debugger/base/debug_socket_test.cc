// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "debugger/base/debug_socket.h"
#include "gtest/gtest.h"

namespace {
const int kListPort = 4014;
const int kWaitMs = 100;

// debug::ListeningSocket test fixture.
class SocketTest : public ::testing::Test {
 public:
  SocketTest() {}
};

// Unit tests start here.
TEST_F(SocketTest, Uninitialized) {
  debug::ListeningSocket sock;
  debug::Socket new_connection;
  EXPECT_FALSE(new_connection.IsConnected());
  EXPECT_FALSE(sock.Accept(kWaitMs, &new_connection));
  EXPECT_FALSE(new_connection.IsConnected());

  char buff[100];
  EXPECT_EQ(0, new_connection.Read(buff, sizeof(buff), kWaitMs));
  EXPECT_EQ(0, new_connection.Write(buff, sizeof(buff), kWaitMs));
  EXPECT_EQ(0, new_connection.WriteAll(buff, sizeof(buff)));
  EXPECT_EQ(0, new_connection.WriteAll(debug::Blob(buff, sizeof(buff))));
  EXPECT_EQ(WSAENOTSOCK, new_connection.GetLastError());
}

bool ListenOnPort(debug::ListeningSocket* list_sock, int* port) {
  int listen_on_port = 0;
  for (int i = 0; i < 20; i++) {
    listen_on_port = kListPort + (i * 2);
    if (list_sock->Listen(listen_on_port)) {
      *port = listen_on_port;
      return true;
    }
  }
  return false;
}

bool CreateSocketPair(debug::Socket* client_sock, debug::Socket* server_sock) {
  debug::ListeningSocket list_sock;
  int port = 0;
  if (!ListenOnPort(&list_sock, &port))
    return false;

  if (!client_sock->ConnectTo("localhost", port))
    return false;

  if (!list_sock.Accept(kWaitMs, server_sock))
    return false;
  return true;
}

TEST_F(SocketTest, Listen) {
  debug::ListeningSocket list_sock;
  int listen_on_port = 0;
  ASSERT_TRUE(ListenOnPort(&list_sock, &listen_on_port));

  // Should fail if try to listen on the same port twice.
  debug::ListeningSocket list_sock2;
  EXPECT_FALSE(list_sock2.Listen(listen_on_port));
  EXPECT_EQ(WSAEADDRINUSE, list_sock2.GetLastError());
}

TEST_F(SocketTest, ListenAndConnect) {
  debug::ListeningSocket list_sock;
  int listen_on_port = 0;
  ASSERT_TRUE(ListenOnPort(&list_sock, &listen_on_port));

  debug::Socket client_sock;
  debug::Socket server_sock;
  ASSERT_TRUE(client_sock.ConnectTo("localhost", listen_on_port));
  ASSERT_TRUE(list_sock.Accept(kWaitMs, &server_sock));
  ASSERT_TRUE(client_sock.IsConnected());
  ASSERT_TRUE(server_sock.IsConnected());
}

TEST_F(SocketTest, SmallClientToServerTransfer) {
  debug::Socket client_sock;
  debug::Socket server_sock;
  ASSERT_TRUE(CreateSocketPair(&client_sock, &server_sock));
  ASSERT_TRUE(client_sock.IsConnected());
  ASSERT_TRUE(server_sock.IsConnected());

  const char kMsgToHost[] = "aaad4h29348dh2";
  char recv_buff[sizeof(kMsgToHost)];

  EXPECT_EQ(sizeof(kMsgToHost),
            client_sock.Write(kMsgToHost, sizeof(kMsgToHost), kWaitMs));
  EXPECT_EQ(0, client_sock.GetLastError());
  EXPECT_EQ(sizeof(kMsgToHost),
            server_sock.Read(recv_buff, sizeof(recv_buff), kWaitMs));
  EXPECT_EQ(0, server_sock.GetLastError());
  EXPECT_STREQ(kMsgToHost, recv_buff);
}

TEST_F(SocketTest, LargeClientToServerTransfer) {
  debug::Socket client_sock;
  debug::Socket server_sock;
  ASSERT_TRUE(CreateSocketPair(&client_sock, &server_sock));
  ASSERT_TRUE(client_sock.IsConnected());
  ASSERT_TRUE(server_sock.IsConnected());

  char send_buff[16 * 1024];
  for (size_t i = 0; i < sizeof(send_buff); i++)
    send_buff[i] = (i % 200) + 1;
  send_buff[sizeof(send_buff) - 1] = 0;
  EXPECT_EQ(sizeof(send_buff),
            client_sock.WriteAll(send_buff, sizeof(send_buff)));
  EXPECT_EQ(0, client_sock.GetLastError());

  char recv_buff[sizeof(send_buff)];
  memset(recv_buff, 0, sizeof(recv_buff));
  EXPECT_EQ(sizeof(recv_buff),
            server_sock.ReadAll(recv_buff, sizeof(recv_buff)));
  EXPECT_EQ(0, server_sock.GetLastError());
  EXPECT_STREQ(send_buff, recv_buff);
}

TEST_F(SocketTest, ServerToClientTransfer) {
  debug::Socket client_sock;
  debug::Socket server_sock;
  ASSERT_TRUE(CreateSocketPair(&client_sock, &server_sock));

  const char kMsgToClient[] = "bbb75c4bf80f643";
  char recv_buff[sizeof(kMsgToClient)];
  EXPECT_EQ(sizeof(kMsgToClient),
            server_sock.Write(kMsgToClient, sizeof(kMsgToClient), kWaitMs));
  EXPECT_EQ(0, server_sock.GetLastError());
  EXPECT_EQ(sizeof(kMsgToClient),
            client_sock.Read(recv_buff, sizeof(recv_buff), kWaitMs));
  EXPECT_EQ(0, client_sock.GetLastError());
  EXPECT_STREQ(kMsgToClient, recv_buff);
}

TEST_F(SocketTest, SendShallFailOnClosedClientSocket) {
  debug::Socket client_sock;
  debug::Socket server_sock;
  ASSERT_TRUE(CreateSocketPair(&client_sock, &server_sock));
  client_sock.Close();
  EXPECT_EQ(0, client_sock.WriteAll("aaa", 3));
  EXPECT_EQ(WSAENOTSOCK, client_sock.GetLastError());
}

}  // namespace

