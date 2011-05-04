// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This application forwards the port that the rsp debug stub opens on the
// loopback interface, and forwards it to the public interface, so that a
// remote debugger can connect to it.

#include <conio.h>

#include "rsp_proxy/proxy_socket.h"

namespace {
enum ConnectionState {CONNECTING, CONNECTED};
const int kBufferSize = 512;
const int kListenMs = 20;
const int kReadWaitMs = 20;
}

void Disconnect(debug::ListeningSocket client_listening_socket,
                debug::Socket client_socket,
                debug::Socket server_socket) {
  client_listening_socket.Close();
  client_socket.Close();
  server_socket.Close();
}

int main(int argc, char **argv) {
  debug::Socket server_side_socket;
  debug::ListeningSocket client_side_listening_socket;
  debug::Socket client_side_socket;
  ConnectionState state = CONNECTING;

  client_side_listening_socket.Setup(4014);

  while (true) {
    if (CONNECTING == state) {
      if (!server_side_socket.IsConnected()) {
        server_side_socket.ConnectTo("127.0.0.1", 4014);
        if (server_side_socket.IsConnected())
          printf("Connected to the debug_stub.\n");
      } else if (!client_side_socket.IsConnected()) {
        client_side_listening_socket.Accept(&client_side_socket, kListenMs);
        if (client_side_socket.IsConnected())
          printf("Got connection from host\n");
      } else {
        state = CONNECTED;
      }
      continue;
    }

    if (CONNECTED == state) {
      if (!server_side_socket.IsConnected() ||
          !client_side_socket.IsConnected()) {
        Disconnect(client_side_listening_socket,
                   client_side_socket,
                   server_side_socket);
        state = CONNECTING;
        continue;
      }
    }

    char buff[kBufferSize];
    size_t read_bytes = client_side_socket.Read(buff,
                                                sizeof(buff) - 1,
                                                kReadWaitMs);
    if (read_bytes > 0) {
      buff[read_bytes] = 0;
      printf("h>%s\n", buff);
      server_side_socket.WriteAll(buff, read_bytes);
    }

    read_bytes = server_side_socket.Read(buff, sizeof(buff) - 1, kReadWaitMs);
    if (read_bytes > 0) {
      buff[read_bytes] = 0;
      printf("t>%s\n", buff);
      client_side_socket.WriteAll(buff, read_bytes);
    }

    if (_kbhit()) {
      char c = getchar();
      if ('q' == c) {
        break;
      }
    }
  }

  if (CONNECTED == state) {
    Disconnect(client_side_listening_socket,
               client_side_socket,
               server_side_socket);
  }

  return 0;
}
